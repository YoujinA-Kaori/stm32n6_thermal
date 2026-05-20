#!/usr/bin/env python3
"""Run TFLite inference on a dataset split and report classification metrics."""

from __future__ import annotations

import argparse
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
from src.metrics import (
    build_collection_suggestions,
    compute_classification_metrics,
    compute_confusion_matrix,
    format_classification_report,
    format_confusion_matrix,
)
from src.model import require_tensorflow
from src.tflite_utils import inspect_tflite_model, load_tflite_interpreter, run_tflite_inference


def save_confusion_csv(confusion: np.ndarray, class_names: list[str], output_path: Path) -> None:
    """Save the confusion matrix as a CSV file."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as fp:
        fp.write("true/pred," + ",".join(class_names) + "\n")
        for class_name, row in zip(class_names, confusion):
            fp.write(class_name + "," + ",".join(str(int(value)) for value in row) + "\n")


def evaluate_tflite(interpreter, sample_records, config: dict[str, Any], class_names: list[str]) -> dict[str, Any]:
    """Run TFLite inference across one split and compute metrics."""
    y_true: list[int] = []
    y_pred: list[int] = []
    confidences: list[float] = []
    heatmap_peak_errors: list[float] = []
    default_sigma_px = float(config["supervision"]["heatmap_sigma_px"])

    for sample in sample_records:
        frame_u16 = load_temp14_frame(sample.bin_path, config)
        frame_input = normalize_temp14_frame(frame_u16, config, add_channel_axis=True)
        outputs = run_tflite_inference(interpreter, frame_input)
        class_probabilities = outputs[0][0]
        heatmap_output = outputs[1][0, ..., 0]
        prediction = int(np.argmax(class_probabilities))
        confidence = float(np.max(class_probabilities))
        y_true.append(int(sample.label_index))
        y_pred.append(prediction)
        confidences.append(confidence)

        annotation = normalize_sidecar_annotation(load_sidecar_annotation(sample.bin_path), default_sigma_px)
        if sample.class_name != "empty" and annotation is not None and annotation.center_x is not None and annotation.center_y is not None:
            peak_y, peak_x = np.unravel_index(int(np.argmax(heatmap_output)), heatmap_output.shape)
            error_px = float(np.sqrt((peak_x - annotation.center_x) ** 2 + (peak_y - annotation.center_y) ** 2))
            heatmap_peak_errors.append(error_px)

    confusion = compute_confusion_matrix(y_true, y_pred, len(class_names))
    metrics = compute_classification_metrics(confusion)
    return {
        "confusion_matrix": confusion,
        "metrics": metrics,
        "y_true": y_true,
        "y_pred": y_pred,
        "mean_confidence": float(np.mean(confidences)) if confidences else 0.0,
        "mean_heatmap_peak_error_px": float(np.mean(heatmap_peak_errors)) if heatmap_peak_errors else None,
    }


def compare_with_keras(model, interpreter, sample_records, config: dict[str, Any]) -> dict[str, Any]:
    """Measure top-1 agreement between Keras and TFLite on one split."""
    agreement_count = 0
    max_abs_delta = 0.0
    mean_abs_deltas: list[float] = []
    keras_heatmap_errors: list[float] = []
    tflite_heatmap_errors: list[float] = []
    default_sigma_px = float(config["supervision"]["heatmap_sigma_px"])

    for sample in sample_records:
        frame_u16 = load_temp14_frame(sample.bin_path, config)
        frame_input = normalize_temp14_frame(frame_u16, config, add_channel_axis=True)
        keras_outputs = model.predict(frame_input[np.newaxis, ...], verbose=0)
        tflite_outputs = run_tflite_inference(interpreter, frame_input)
        keras_class_output = keras_outputs[0][0]
        keras_heatmap_output = keras_outputs[1][0, ..., 0]
        tflite_class_output = tflite_outputs[0][0]
        tflite_heatmap_output = tflite_outputs[1][0, ..., 0]
        if int(np.argmax(keras_class_output)) == int(np.argmax(tflite_class_output)):
            agreement_count += 1
        abs_delta = np.abs(keras_class_output - tflite_class_output)
        max_abs_delta = max(max_abs_delta, float(np.max(abs_delta)))
        mean_abs_deltas.append(float(np.mean(abs_delta)))

        annotation = normalize_sidecar_annotation(load_sidecar_annotation(sample.bin_path), default_sigma_px)
        if sample.class_name != "empty" and annotation is not None and annotation.center_x is not None and annotation.center_y is not None:
            keras_peak_y, keras_peak_x = np.unravel_index(int(np.argmax(keras_heatmap_output)), keras_heatmap_output.shape)
            tflite_peak_y, tflite_peak_x = np.unravel_index(int(np.argmax(tflite_heatmap_output)), tflite_heatmap_output.shape)
            keras_error = float(np.sqrt((keras_peak_x - annotation.center_x) ** 2 + (keras_peak_y - annotation.center_y) ** 2))
            tflite_error = float(np.sqrt((tflite_peak_x - annotation.center_x) ** 2 + (tflite_peak_y - annotation.center_y) ** 2))
            keras_heatmap_errors.append(keras_error)
            tflite_heatmap_errors.append(tflite_error)

    total = len(sample_records)
    return {
        "samples": total,
        "top1_agreement": float(agreement_count / total) if total > 0 else 0.0,
        "max_abs_delta": max_abs_delta,
        "mean_abs_delta": float(np.mean(mean_abs_deltas)) if mean_abs_deltas else 0.0,
        "mean_keras_heatmap_peak_error_px": float(np.mean(keras_heatmap_errors)) if keras_heatmap_errors else None,
        "mean_tflite_heatmap_peak_error_px": float(np.mean(tflite_heatmap_errors)) if tflite_heatmap_errors else None,
    }


def build_arg_parser() -> argparse.ArgumentParser:
    """Build the CLI argument parser."""
    parser = argparse.ArgumentParser(description="Validate a TFLite model on a processed thermal split")
    parser.add_argument("--config", type=Path, default=None, help="Path to dataset_config.json")
    parser.add_argument("--processed-root", type=Path, default=None, help="Processed dataset root, default from config")
    parser.add_argument("--model", type=Path, required=True, help="TFLite model path")
    parser.add_argument("--keras-model", type=Path, default=None, help="Optional Keras model path for consistency checks")
    parser.add_argument("--split", choices=("train", "val", "test"), default="test", help="Dataset split to validate")
    parser.add_argument("--report-dir", type=Path, default=None, help="Validation report output directory")
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
    report_dir = (
        args.report_dir
        if args.report_dir is not None
        else resolve_workspace_path(dataset_config["artifacts"]["report_dir"]) / "validation" / f"{args.model.stem}_{args.split}"
    )
    ensure_dir(report_dir)

    if not args.model.exists():
        raise SystemExit(f"TFLite model not found: {args.model}")

    sample_records = scan_processed_split(processed_root, args.split, class_names)
    if not sample_records:
        raise SystemExit(f"no samples found in split: {args.split}")

    interpreter = load_tflite_interpreter(args.model)
    evaluation = evaluate_tflite(interpreter, sample_records, dataset_config, class_names)
    confusion = evaluation["confusion_matrix"]
    report_text = format_classification_report(class_names, confusion)
    matrix_text = format_confusion_matrix(class_names, confusion)
    suggestions = build_collection_suggestions(class_names, confusion)
    tflite_meta = inspect_tflite_model(args.model)

    (report_dir / "classification_report.txt").write_text(report_text + "\n", encoding="utf-8")
    (report_dir / "confusion_matrix.txt").write_text(matrix_text + "\n", encoding="utf-8")
    (report_dir / "collection_suggestions.txt").write_text("\n".join(suggestions) + ("\n" if suggestions else ""), encoding="utf-8")
    save_confusion_csv(confusion, class_names, report_dir / "confusion_matrix.csv")

    report_payload: dict[str, Any] = {
        "tflite_model": str(args.model),
        "tflite_io": tflite_meta,
        "split": args.split,
        "metrics": evaluation["metrics"],
        "mean_confidence": evaluation["mean_confidence"],
        "mean_heatmap_peak_error_px": evaluation["mean_heatmap_peak_error_px"],
        "collection_suggestions": suggestions,
    }

    if args.keras_model is not None:
        if not args.keras_model.exists():
            raise SystemExit(f"Keras model not found: {args.keras_model}")
        keras_model = tf_module.keras.models.load_model(args.keras_model)
        report_payload["keras_consistency"] = compare_with_keras(keras_model, interpreter, sample_records, dataset_config)

    save_json(report_dir / "validation_summary.json", make_json_safe(report_payload))

    print(matrix_text)
    print("")
    print(report_text)
    print("")
    print(f"mean confidence: {evaluation['mean_confidence']:.4f}")
    print(f"tflite inputs:  {tflite_meta['inputs']}")
    print(f"tflite outputs: {tflite_meta['outputs']}")
    if evaluation["mean_heatmap_peak_error_px"] is not None:
        print(f"mean heatmap peak error px: {evaluation['mean_heatmap_peak_error_px']:.2f}")
    if "keras_consistency" in report_payload:
        print(f"keras/tflite top1 agreement: {report_payload['keras_consistency']['top1_agreement']:.4f}")
        if report_payload["keras_consistency"]["mean_keras_heatmap_peak_error_px"] is not None:
            print(f"keras baseline heatmap error px: {report_payload['keras_consistency']['mean_keras_heatmap_peak_error_px']:.2f}")
        if report_payload["keras_consistency"]["mean_tflite_heatmap_peak_error_px"] is not None:
            print(f"tflite heatmap error px: {report_payload['keras_consistency']['mean_tflite_heatmap_peak_error_px']:.2f}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
