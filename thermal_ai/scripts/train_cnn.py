#!/usr/bin/env python3
"""Train the lightweight five-class CNN on processed temp14 frames."""

from __future__ import annotations

import argparse
import random
from pathlib import Path
from typing import Any

import numpy as np

import _bootstrap

_bootstrap.setup_python_path()

from src.common import (
    build_gaussian_heatmap,
    ensure_dir,
    get_class_names,
    get_frame_shape,
    get_input_shape,
    load_dataset_config,
    load_temp14_frame,
    load_train_config,
    load_sidecar_annotation,
    make_json_safe,
    normalize_temp14_frame,
    resolve_workspace_path,
    normalize_sidecar_annotation,
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
from src.model import build_cnn_model, require_tensorflow


def set_global_seed(seed: int) -> None:
    """Seed Python, NumPy, and TensorFlow RNGs for reproducibility."""
    tf_module = require_tensorflow()
    random.seed(seed)
    np.random.seed(seed)
    tf_module.random.set_seed(seed)


def build_tf_dataset(
    sample_records,
    config: dict[str, Any],
    batch_size: int,
    shuffle: bool,
    seed: int,
):
    """Build a tf.data pipeline that reads raw temp14 .bin files on the fly."""
    tf_module = require_tensorflow()
    records = list(sample_records)
    if not records:
        raise ValueError("dataset split is empty")

    if shuffle:
        rng = random.Random(seed)
        rng.shuffle(records)

    height, width, channels = get_input_shape(config)
    heatmap_height, heatmap_width = get_frame_shape(config)
    default_sigma_px = float(config["supervision"]["heatmap_sigma_px"])

    def validate_sample_annotation(sample) -> None:
        """Ensure a sample has a usable center-point annotation when required."""
        annotation_payload = load_sidecar_annotation(sample.bin_path)
        annotation = normalize_sidecar_annotation(annotation_payload, default_sigma_px)
        if sample.class_name == "empty":
            return
        if annotation is None or annotation.center_x is None or annotation.center_y is None:
            raise SystemExit(
                f"missing heatmap annotation for non-empty sample: {sample.bin_path} "
                f"(expected a JSON sidecar with center_x/center_y)"
            )
        if annotation.class_name is not None and annotation.class_name != sample.class_name:
            raise SystemExit(
                f"annotation class mismatch for {sample.bin_path}: "
                f"sample={sample.class_name}, annotation={annotation.class_name}"
            )

    for sample in records:
        validate_sample_annotation(sample)

    def generator():
        """Yield normalized frames and multi-task labels sample by sample."""
        for sample in records:
            frame_u16 = load_temp14_frame(sample.bin_path, config)
            frame_input = normalize_temp14_frame(frame_u16, config, add_channel_axis=True).astype(np.float32)
            annotation_payload = load_sidecar_annotation(sample.bin_path)
            annotation = normalize_sidecar_annotation(annotation_payload, default_sigma_px)

            if sample.class_name == "empty":
                heatmap = np.zeros((heatmap_height, heatmap_width, 1), dtype=np.float32)
            else:
                heatmap_2d = build_gaussian_heatmap(
                    heatmap_height,
                    heatmap_width,
                    annotation.center_x if annotation is not None else None,
                    annotation.center_y if annotation is not None else None,
                    annotation.sigma_px if annotation is not None else default_sigma_px,
                )
                heatmap = heatmap_2d[..., np.newaxis].astype(np.float32)

            yield (
                frame_input,
                {
                    "class_probs": np.int32(sample.label_index),
                    "heatmap": heatmap,
                },
            )

    dataset = tf_module.data.Dataset.from_generator(
        generator,
        output_signature=(
            tf_module.TensorSpec(shape=(height, width, channels), dtype=tf_module.float32),
            {
                "class_probs": tf_module.TensorSpec(shape=(), dtype=tf_module.int32),
                "heatmap": tf_module.TensorSpec(shape=(heatmap_height, heatmap_width, 1), dtype=tf_module.float32),
            },
        ),
    )
    if shuffle:
        dataset = dataset.shuffle(buffer_size=len(records), seed=seed, reshuffle_each_iteration=True)
    dataset = dataset.batch(batch_size)
    dataset = dataset.prefetch(tf_module.data.AUTOTUNE)
    return dataset


def save_confusion_csv(confusion: np.ndarray, class_names: list[str], output_path: Path) -> None:
    """Save the confusion matrix as a CSV table."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with output_path.open("w", encoding="utf-8", newline="") as fp:
        fp.write("true/pred," + ",".join(class_names) + "\n")
        for class_name, row in zip(class_names, confusion):
            fp.write(class_name + "," + ",".join(str(int(value)) for value in row) + "\n")


def evaluate_split(model, dataset, sample_records, split_name: str, class_names: list[str], config: dict[str, Any]) -> dict[str, Any]:
    """Evaluate one dataset split and collect full metrics."""
    evaluation = model.evaluate(dataset, verbose=0, return_dict=True)
    predictions = model.predict(dataset, verbose=0)
    class_probabilities = predictions[0]
    heatmap_predictions = predictions[1]
    y_pred = np.argmax(class_probabilities, axis=1).tolist()
    y_true = [int(sample.label_index) for sample in sample_records]
    confusion = compute_confusion_matrix(y_true, y_pred, len(class_names))
    metrics = compute_classification_metrics(confusion)
    default_sigma_px = float(config["supervision"]["heatmap_sigma_px"])
    localization_errors: list[float] = []
    class_accuracy = float(evaluation.get("class_probs_accuracy", evaluation.get("accuracy", 0.0)))
    heatmap_mae = float(evaluation.get("heatmap_mae", evaluation.get("heatmap_mean_absolute_error", 0.0)))

    for sample, heatmap_prediction in zip(sample_records, heatmap_predictions):
        if sample.class_name == "empty":
            continue
        annotation = normalize_sidecar_annotation(load_sidecar_annotation(sample.bin_path), default_sigma_px)
        if annotation is None or annotation.center_x is None or annotation.center_y is None:
            continue

        peak_index = int(np.argmax(heatmap_prediction[..., 0]))
        peak_y, peak_x = np.unravel_index(peak_index, heatmap_prediction[..., 0].shape)
        error_px = float(np.sqrt((peak_x - annotation.center_x) ** 2 + (peak_y - annotation.center_y) ** 2))
        localization_errors.append(error_px)

    return {
        "split": split_name,
        "loss": float(evaluation.get("loss", 0.0)),
        "class_accuracy": class_accuracy,
        "heatmap_mae": heatmap_mae,
        "mean_localization_error_px": float(np.mean(localization_errors)) if localization_errors else None,
        "localization_samples": len(localization_errors),
        "confusion_matrix": confusion,
        "metrics": metrics,
        "y_true": y_true,
        "y_pred": y_pred,
    }


def build_arg_parser() -> argparse.ArgumentParser:
    """Build the CLI argument parser."""
    parser = argparse.ArgumentParser(description="Train the STM32N6 thermal five-class classifier")
    parser.add_argument("--config", type=Path, default=None, help="Path to dataset_config.json")
    parser.add_argument("--train-config", type=Path, default=None, help="Path to train_config.json")
    parser.add_argument("--processed-root", type=Path, default=None, help="Processed dataset root, default from config")
    parser.add_argument("--output-dir", type=Path, default=None, help="Model artifact output directory")
    parser.add_argument("--report-dir", type=Path, default=None, help="Training report output directory")
    return parser


def main() -> int:
    """Program entry point."""
    tf_module = require_tensorflow()
    args = build_arg_parser().parse_args()
    dataset_config = load_dataset_config(args.config)
    train_config = load_train_config(args.train_config)
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
    report_dir = (
        args.report_dir
        if args.report_dir is not None
        else resolve_workspace_path(dataset_config["artifacts"]["report_dir"]) / "training"
    )
    ensure_dir(output_dir)
    ensure_dir(report_dir)

    seed = int(train_config["seed"])
    batch_size = int(train_config["batch_size"])
    epochs = int(train_config["epochs"])
    set_global_seed(seed)

    train_samples = scan_processed_split(processed_root, "train", class_names)
    val_samples = scan_processed_split(processed_root, "val", class_names)
    test_samples = scan_processed_split(processed_root, "test", class_names)
    if not train_samples or not val_samples or not test_samples:
        raise SystemExit("train/val/test splits must all contain samples before training")

    train_dataset = build_tf_dataset(train_samples, dataset_config, batch_size, shuffle=True, seed=seed)
    train_eval_dataset = build_tf_dataset(train_samples, dataset_config, batch_size, shuffle=False, seed=seed)
    val_dataset = build_tf_dataset(val_samples, dataset_config, batch_size, shuffle=False, seed=seed)
    test_dataset = build_tf_dataset(test_samples, dataset_config, batch_size, shuffle=False, seed=seed)

    model = build_cnn_model(dataset_config, train_config)
    best_model_path = output_dir / "best_model.keras"
    final_model_path = output_dir / "final_model.keras"

    callbacks = [
        tf_module.keras.callbacks.ModelCheckpoint(
            filepath=str(best_model_path),
            monitor="val_class_probs_accuracy",
            mode="max",
            save_best_only=True,
        ),
        tf_module.keras.callbacks.EarlyStopping(
            monitor="val_class_probs_accuracy",
            mode="max",
            patience=int(train_config["early_stopping_patience"]),
            restore_best_weights=False,
        ),
        tf_module.keras.callbacks.ReduceLROnPlateau(
            monitor="val_loss",
            factor=float(train_config["reduce_lr_factor"]),
            patience=int(train_config["reduce_lr_patience"]),
            min_lr=float(train_config["min_learning_rate"]),
            verbose=1,
        ),
    ]

    history = model.fit(
        train_dataset,
        validation_data=val_dataset,
        epochs=epochs,
        callbacks=callbacks,
        verbose=1,
    )

    model.save(final_model_path)
    best_model = tf_module.keras.models.load_model(best_model_path)

    train_result = evaluate_split(best_model, train_eval_dataset, train_samples, "train", class_names)
    val_result = evaluate_split(best_model, val_dataset, val_samples, "val", class_names)
    test_result = evaluate_split(best_model, test_dataset, test_samples, "test", class_names)

    save_json(output_dir / "class_names.json", class_names)

    confusion = test_result["confusion_matrix"]
    report_text = format_classification_report(class_names, confusion)
    matrix_text = format_confusion_matrix(class_names, confusion)
    suggestions = build_collection_suggestions(class_names, confusion)

    (report_dir / "classification_report.txt").write_text(report_text + "\n", encoding="utf-8")
    (report_dir / "confusion_matrix.txt").write_text(matrix_text + "\n", encoding="utf-8")
    (report_dir / "collection_suggestions.txt").write_text("\n".join(suggestions) + ("\n" if suggestions else ""), encoding="utf-8")
    save_confusion_csv(confusion, class_names, report_dir / "confusion_matrix.csv")
    localization_report_lines = [
        f"train_localization_error_px: {train_result['mean_localization_error_px']}",
        f"val_localization_error_px: {val_result['mean_localization_error_px']}",
        f"test_localization_error_px: {test_result['mean_localization_error_px']}",
        f"train_heatmap_mae: {train_result['heatmap_mae']:.6f}",
        f"val_heatmap_mae: {val_result['heatmap_mae']:.6f}",
        f"test_heatmap_mae: {test_result['heatmap_mae']:.6f}",
    ]
    (report_dir / "localization_report.txt").write_text("\n".join(localization_report_lines) + "\n", encoding="utf-8")

    summary_payload = {
        "history": history.history,
        "train_class_accuracy": train_result["class_accuracy"],
        "val_class_accuracy": val_result["class_accuracy"],
        "test_class_accuracy": test_result["class_accuracy"],
        "train_loss": train_result["loss"],
        "val_loss": val_result["loss"],
        "test_loss": test_result["loss"],
        "train_heatmap_mae": train_result["heatmap_mae"],
        "val_heatmap_mae": val_result["heatmap_mae"],
        "test_heatmap_mae": test_result["heatmap_mae"],
        "train_localization_error_px": train_result["mean_localization_error_px"],
        "val_localization_error_px": val_result["mean_localization_error_px"],
        "test_localization_error_px": test_result["mean_localization_error_px"],
        "test_metrics": test_result["metrics"],
        "collection_suggestions": suggestions,
        "class_names": class_names,
        "best_model_path": str(best_model_path),
        "final_model_path": str(final_model_path),
    }
    save_json(report_dir / "training_summary.json", make_json_safe(summary_payload))

    print(f"train class accuracy: {train_result['class_accuracy']:.4f}")
    print(f"val class accuracy:   {val_result['class_accuracy']:.4f}")
    print(f"test class accuracy:  {test_result['class_accuracy']:.4f}")
    print(f"test heatmap mae:     {test_result['heatmap_mae']:.4f}")
    if test_result["mean_localization_error_px"] is not None:
        print(f"test localization error px: {test_result['mean_localization_error_px']:.2f}")
    print("")
    print(matrix_text)
    print("")
    print(report_text)
    if test_result["mean_localization_error_px"] is not None:
        print("")
        print(f"heatmap peak localization error px: {test_result['mean_localization_error_px']:.2f}")
    if suggestions:
        print("")
        print("data collection suggestions:")
        for line in suggestions:
            print(f"- {line}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
