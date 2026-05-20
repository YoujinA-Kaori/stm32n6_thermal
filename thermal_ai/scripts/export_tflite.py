#!/usr/bin/env python3
"""Export float32 and int8 TFLite models and compare them with Keras."""

from __future__ import annotations

import argparse
import random
from collections import defaultdict
from pathlib import Path
from typing import Any

import numpy as np

import _bootstrap

_bootstrap.setup_python_path()

from src.common import (
    ensure_dir,
    get_class_names,
    load_dataset_config,
    load_temp14_frame,
    load_sidecar_annotation,
    make_json_safe,
    normalize_temp14_frame,
    normalize_sidecar_annotation,
    resolve_workspace_path,
    save_json,
    scan_processed_split,
)
from src.model import require_tensorflow
from src.tflite_utils import inspect_tflite_model, load_tflite_interpreter, run_tflite_inference


def select_balanced_samples(sample_records, class_names: list[str], limit: int, seed: int):
    """Select a class-balanced sample subset for calibration or comparison."""
    rng = random.Random(seed)
    buckets: dict[str, list[Any]] = defaultdict(list)
    for sample in sample_records:
        buckets[sample.class_name].append(sample)

    for class_name in class_names:
        rng.shuffle(buckets[class_name])

    selected = []
    while len(selected) < limit:
        progress = False
        for class_name in class_names:
            if buckets[class_name]:
                selected.append(buckets[class_name].pop())
                progress = True
                if len(selected) >= limit:
                    break
        if not progress:
            break

    return selected


def representative_dataset_generator(sample_records, config: dict[str, Any]):
    """Yield normalized sample tensors for int8 representative calibration."""
    for sample in sample_records:
        frame_u16 = load_temp14_frame(sample.bin_path, config)
        frame_input = normalize_temp14_frame(frame_u16, config, add_channel_axis=True)
        yield [frame_input[np.newaxis, ...].astype(np.float32)]


def compare_keras_and_tflite(model, tflite_path: Path, sample_records, config: dict[str, Any]) -> dict[str, Any]:
    """Compare top-1 agreement and output deltas between Keras and TFLite."""
    interpreter = load_tflite_interpreter(tflite_path)
    agreements = 0
    max_abs_delta = 0.0
    mean_abs_deltas: list[float] = []
    keras_heatmap_errors: list[float] = []
    tflite_heatmap_errors: list[float] = []
    default_sigma_px = float(config["supervision"]["heatmap_sigma_px"])

    for sample in sample_records:
        frame_u16 = load_temp14_frame(sample.bin_path, config)
        frame_input = normalize_temp14_frame(frame_u16, config, add_channel_axis=True)
        keras_outputs = model.predict(frame_input[np.newaxis, ...], verbose=0)
        keras_class_output = keras_outputs[0][0]
        keras_heatmap_output = keras_outputs[1][0, ..., 0]
        tflite_outputs = run_tflite_inference(interpreter, frame_input)
        tflite_class_output = tflite_outputs[0][0]
        tflite_heatmap_output = tflite_outputs[1][0, ..., 0]
        if int(np.argmax(keras_class_output)) == int(np.argmax(tflite_class_output)):
            agreements += 1
        abs_delta = np.abs(keras_class_output - tflite_class_output)
        max_abs_delta = max(max_abs_delta, float(np.max(abs_delta)))
        mean_abs_deltas.append(float(np.mean(abs_delta)))

        annotation = normalize_sidecar_annotation(load_sidecar_annotation(sample.bin_path), default_sigma_px)
        if sample.class_name != "empty" and annotation is not None and annotation.center_x is not None and annotation.center_y is not None:
            keras_peak = np.unravel_index(int(np.argmax(keras_heatmap_output)), keras_heatmap_output.shape)
            tflite_peak = np.unravel_index(int(np.argmax(tflite_heatmap_output)), tflite_heatmap_output.shape)
            keras_error = float(np.sqrt((keras_peak[1] - annotation.center_x) ** 2 + (keras_peak[0] - annotation.center_y) ** 2))
            tflite_error = float(np.sqrt((tflite_peak[1] - annotation.center_x) ** 2 + (tflite_peak[0] - annotation.center_y) ** 2))
            keras_heatmap_errors.append(keras_error)
            tflite_heatmap_errors.append(tflite_error)

    total = len(sample_records)
    return {
        "samples": total,
        "top1_agreement": float(agreements / total) if total > 0 else 0.0,
        "max_abs_delta": max_abs_delta,
        "mean_abs_delta": float(np.mean(mean_abs_deltas)) if mean_abs_deltas else 0.0,
        "mean_keras_heatmap_peak_error_px": float(np.mean(keras_heatmap_errors)) if keras_heatmap_errors else None,
        "mean_tflite_heatmap_peak_error_px": float(np.mean(tflite_heatmap_errors)) if tflite_heatmap_errors else None,
    }


def build_arg_parser() -> argparse.ArgumentParser:
    """Build the CLI argument parser."""
    parser = argparse.ArgumentParser(description="Export float32 and int8 TFLite models")
    parser.add_argument("--config", type=Path, default=None, help="Path to dataset_config.json")
    parser.add_argument("--processed-root", type=Path, default=None, help="Processed dataset root, default from config")
    parser.add_argument("--model", type=Path, default=None, help="Keras model path, default best_model.keras")
    parser.add_argument("--output-dir", type=Path, default=None, help="TFLite artifact output directory")
    parser.add_argument("--report-dir", type=Path, default=None, help="TFLite report output directory")
    parser.add_argument("--representative-count", type=int, default=128, help="Representative dataset sample count")
    parser.add_argument("--compare-count", type=int, default=64, help="Comparison sample count")
    parser.add_argument("--seed", type=int, default=42, help="Sampling seed")
    return parser


def main() -> int:
    """Program entry point."""
    tf_module = require_tensorflow()
    args = build_arg_parser().parse_args()
    dataset_config = load_dataset_config(args.config)
    class_names = get_class_names(dataset_config)

    processed_root = (
        args.processed_root
        if args.processed_root is not None
        else resolve_workspace_path(dataset_config["dataset_paths"]["processed_root"])
    )
    output_dir = (
        args.output_dir
        if args.output_dir is not None
        else resolve_workspace_path(dataset_config["artifacts"]["model_dir"])
    )
    model_path = args.model if args.model is not None else output_dir / "best_model.keras"
    report_dir = (
        args.report_dir
        if args.report_dir is not None
        else resolve_workspace_path(dataset_config["artifacts"]["report_dir"]) / "tflite" / model_path.stem
    )
    ensure_dir(output_dir)
    ensure_dir(report_dir)

    if not model_path.exists():
        raise SystemExit(f"keras model not found: {model_path}")

    model = tf_module.keras.models.load_model(model_path)
    train_samples = scan_processed_split(processed_root, "train", class_names)
    test_samples = scan_processed_split(processed_root, "test", class_names)
    if not train_samples or not test_samples:
        raise SystemExit("train and test splits must exist before TFLite export")

    representative_samples = select_balanced_samples(train_samples, class_names, args.representative_count, args.seed)
    comparison_samples = select_balanced_samples(test_samples, class_names, args.compare_count, args.seed)

    float_tflite_path = output_dir / f"{model_path.stem}_float32.tflite"
    float_converter = tf_module.lite.TFLiteConverter.from_keras_model(model)
    float_tflite_model = float_converter.convert()
    float_tflite_path.write_bytes(float_tflite_model)

    int8_tflite_path = output_dir / f"{model_path.stem}_int8.tflite"
    int8_converter = tf_module.lite.TFLiteConverter.from_keras_model(model)
    int8_converter.optimizations = [tf_module.lite.Optimize.DEFAULT]
    int8_converter.representative_dataset = lambda: representative_dataset_generator(representative_samples, dataset_config)
    int8_converter.target_spec.supported_ops = [tf_module.lite.OpsSet.TFLITE_BUILTINS_INT8]
    int8_converter.inference_input_type = tf_module.int8
    int8_converter.inference_output_type = tf_module.int8
    int8_tflite_model = int8_converter.convert()
    int8_tflite_path.write_bytes(int8_tflite_model)

    float_meta = inspect_tflite_model(float_tflite_path)
    int8_meta = inspect_tflite_model(int8_tflite_path)
    float_compare = compare_keras_and_tflite(model, float_tflite_path, comparison_samples, dataset_config)
    int8_compare = compare_keras_and_tflite(model, int8_tflite_path, comparison_samples, dataset_config)

    report_payload = {
        "keras_model": str(model_path),
        "float32_tflite": float_meta,
        "int8_tflite": int8_meta,
        "float32_comparison": float_compare,
        "int8_comparison": int8_compare,
        "representative_count": len(representative_samples),
        "comparison_count": len(comparison_samples),
    }
    save_json(report_dir / "tflite_export_report.json", make_json_safe(report_payload))

    print(f"float32 tflite: {float_tflite_path}")
    print(f"int8 tflite:    {int8_tflite_path}")
    print("")
    print("float32 input/output:")
    print(float_meta)
    print("")
    print("int8 input/output:")
    print(int8_meta)
    print("")
    print(f"float32 top1 agreement: {float_compare['top1_agreement']:.4f}")
    print(f"int8 top1 agreement:    {int8_compare['top1_agreement']:.4f}")
    if float_compare["mean_keras_heatmap_peak_error_px"] is not None:
        print(f"keras baseline heatmap error px: {float_compare['mean_keras_heatmap_peak_error_px']:.2f}")
    if float_compare["mean_tflite_heatmap_peak_error_px"] is not None:
        print(f"float32 tflite heatmap error px: {float_compare['mean_tflite_heatmap_peak_error_px']:.2f}")
    if int8_compare["mean_tflite_heatmap_peak_error_px"] is not None:
        print(f"int8 heatmap error px:    {int8_compare['mean_tflite_heatmap_peak_error_px']:.2f}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
