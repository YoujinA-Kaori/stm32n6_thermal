#!/usr/bin/env python3
"""Metrics helpers for five-class thermal image classification."""

from __future__ import annotations

from typing import Any

import numpy as np


def compute_confusion_matrix(
    y_true: list[int] | np.ndarray,
    y_pred: list[int] | np.ndarray,
    num_classes: int,
) -> np.ndarray:
    """Build a dense confusion matrix from integer labels."""
    confusion = np.zeros((num_classes, num_classes), dtype=np.int64)
    for true_label, pred_label in zip(y_true, y_pred):
        confusion[int(true_label), int(pred_label)] += 1
    return confusion


def compute_classification_metrics(confusion: np.ndarray) -> dict[str, Any]:
    """Compute accuracy plus per-class precision, recall, and F1-score."""
    support = confusion.sum(axis=1)
    predicted = confusion.sum(axis=0)
    diagonal = np.diag(confusion)

    with np.errstate(divide="ignore", invalid="ignore"):
        precision = np.divide(diagonal, predicted, out=np.zeros_like(diagonal, dtype=np.float64), where=predicted > 0)
        recall = np.divide(diagonal, support, out=np.zeros_like(diagonal, dtype=np.float64), where=support > 0)
        f1_score = np.divide(
            2.0 * precision * recall,
            precision + recall,
            out=np.zeros_like(precision, dtype=np.float64),
            where=(precision + recall) > 0,
        )

    total = int(confusion.sum())
    accuracy = float(diagonal.sum() / total) if total > 0 else 0.0

    macro_precision = float(np.mean(precision)) if precision.size else 0.0
    macro_recall = float(np.mean(recall)) if recall.size else 0.0
    macro_f1_score = float(np.mean(f1_score)) if f1_score.size else 0.0

    return {
        "accuracy": accuracy,
        "macro_precision": macro_precision,
        "macro_recall": macro_recall,
        "macro_f1_score": macro_f1_score,
        "per_class": [
            {
                "precision": float(precision[index]),
                "recall": float(recall[index]),
                "f1_score": float(f1_score[index]),
                "support": int(support[index]),
            }
            for index in range(confusion.shape[0])
        ],
    }


def format_confusion_matrix(class_names: list[str], confusion: np.ndarray) -> str:
    """Format the confusion matrix as an aligned plain-text table."""
    column_width = max(len(name) for name in class_names + ["true/pred"]) + 2
    header = "true/pred".ljust(column_width) + "".join(name.ljust(column_width) for name in class_names)
    rows = [header]

    for row_index, class_name in enumerate(class_names):
        row_values = "".join(str(int(value)).ljust(column_width) for value in confusion[row_index])
        rows.append(class_name.ljust(column_width) + row_values)

    return "\n".join(rows)


def format_classification_report(class_names: list[str], confusion: np.ndarray) -> str:
    """Render the classification metrics into a plain-text report."""
    metrics = compute_classification_metrics(confusion)
    lines = []
    lines.append("class".ljust(28) + "precision  recall     f1-score   support")

    for class_name, class_metric in zip(class_names, metrics["per_class"]):
        lines.append(
            class_name.ljust(28)
            + f"{class_metric['precision']:9.4f}"
            + f"{class_metric['recall']:10.4f}"
            + f"{class_metric['f1_score']:11.4f}"
            + f"{class_metric['support']:10d}"
        )

    lines.append("")
    lines.append(f"accuracy{'':20}{metrics['accuracy']:.4f}")
    lines.append(f"macro avg{'':17}{metrics['macro_precision']:.4f}    {metrics['macro_recall']:.4f}    {metrics['macro_f1_score']:.4f}")
    return "\n".join(lines)


def build_collection_suggestions(class_names: list[str], confusion: np.ndarray) -> list[str]:
    """Generate data-collection suggestions when confusion is high."""
    suggestions: list[str] = []
    metrics = compute_classification_metrics(confusion)
    accuracy = metrics["accuracy"]

    if accuracy >= 0.9:
        return suggestions

    for source_index, class_metric in enumerate(metrics["per_class"]):
        support = int(class_metric["support"])
        if support <= 0:
            continue

        row = confusion[source_index].copy()
        row[source_index] = 0
        target_index = int(np.argmax(row))
        mistake_count = int(row[target_index])
        mistake_ratio = float(mistake_count / support) if support > 0 else 0.0
        if mistake_count <= 0 or mistake_ratio < 0.15:
            continue

        pair = {class_names[source_index], class_names[target_index]}
        if pair == {"person", "hand"}:
            suggestions.append(
                "person and hand confusion is high; add more distance, pose, and partial-occlusion sessions for both classes."
            )
        elif pair == {"hot_object", "circuit_board_hotspot"}:
            suggestions.append(
                "hot_object and circuit_board_hotspot confusion is high; add more board-only hotspot scenes and isolated hot-object scenes."
            )
        elif class_names[source_index] == "empty":
            suggestions.append(
                "empty is being confused with target classes; add more background-only sessions across different ambient temperatures."
            )
        else:
            suggestions.append(
                f"{class_names[source_index]} is often predicted as {class_names[target_index]}; collect more session-diverse samples for this pair."
            )

    if not suggestions and accuracy < 0.9:
        suggestions.append("overall accuracy is still low; increase session diversity and rebalance the raw dataset before the next training run.")

    return suggestions
