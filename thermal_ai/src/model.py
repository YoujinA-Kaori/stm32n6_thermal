#!/usr/bin/env python3
"""Model definition for the lightweight STM32N6 thermal detector."""

from __future__ import annotations

from typing import Any

try:
    import tensorflow as tf
except ImportError as exc:  # pragma: no cover
    tf = None
    _tf_import_error = exc
else:
    _tf_import_error = None


def require_tensorflow():
    """Return the TensorFlow module or stop with a clear error message."""
    if tf is None:  # pragma: no cover
        raise SystemExit(
            "TensorFlow is required for training/export. "
            f"Install it in the active Python environment first: {_tf_import_error}"
        )
    return tf


def _safe_positive_denominator(mask_tensor):
    """Return a stable denominator for positive-cell masked losses."""
    tf_module = require_tensorflow()
    return tf_module.maximum(tf_module.reduce_sum(mask_tensor), tf_module.constant(1.0, dtype=tf_module.float32))

if tf is not None:
    _register_serializable = tf.keras.utils.register_keras_serializable(package="thermal_ai")
else:  # pragma: no cover
    def _register_serializable(function):
        return function


@_register_serializable
def detection_loss(y_true, y_pred):
    """Combined objectness + bbox + class loss for one anchor-free detector head."""
    tf_module = require_tensorflow()
    objectness_true = y_true[..., 0:1]
    bbox_true = y_true[..., 1:5]
    class_true = y_true[..., 5:]

    objectness_logits = y_pred[..., 0:1]
    bbox_logits = y_pred[..., 1:5]
    class_logits = y_pred[..., 5:]

    objectness_loss = tf_module.nn.sigmoid_cross_entropy_with_logits(labels=objectness_true, logits=objectness_logits)
    objectness_loss = tf_module.reduce_mean(objectness_loss)

    positive_mask = objectness_true
    positive_denominator = _safe_positive_denominator(positive_mask) * tf_module.constant(4.0, dtype=tf_module.float32)
    bbox_pred = tf_module.nn.sigmoid(bbox_logits)
    bbox_loss = tf_module.keras.losses.huber(bbox_true, bbox_pred)
    bbox_loss = tf_module.reduce_sum(bbox_loss * positive_mask) / positive_denominator

    class_loss = tf_module.nn.sigmoid_cross_entropy_with_logits(labels=class_true, logits=class_logits)
    class_denominator = _safe_positive_denominator(positive_mask) * tf_module.cast(tf_module.shape(class_true)[-1], tf_module.float32)
    class_loss = tf_module.reduce_sum(class_loss * positive_mask) / class_denominator

    return (
        tf_module.constant(1.0, dtype=tf_module.float32) * objectness_loss
        + tf_module.constant(2.0, dtype=tf_module.float32) * bbox_loss
        + tf_module.constant(1.0, dtype=tf_module.float32) * class_loss
    )


@_register_serializable
def detection_objectness_accuracy(y_true, y_pred):
    """Binary objectness accuracy across all detector cells."""
    tf_module = require_tensorflow()
    objectness_true = y_true[..., 0:1]
    objectness_pred = tf_module.cast(tf_module.nn.sigmoid(y_pred[..., 0:1]) >= 0.5, tf_module.float32)
    return tf_module.reduce_mean(tf_module.cast(tf_module.equal(objectness_true, objectness_pred), tf_module.float32))


@_register_serializable
def detection_bbox_mae(y_true, y_pred):
    """Positive-cell bbox MAE on normalized detector outputs."""
    tf_module = require_tensorflow()
    positive_mask = y_true[..., 0:1]
    bbox_true = y_true[..., 1:5]
    bbox_pred = tf_module.nn.sigmoid(y_pred[..., 1:5])
    absolute_error = tf_module.abs(bbox_true - bbox_pred) * positive_mask
    denominator = _safe_positive_denominator(positive_mask) * tf_module.constant(4.0, dtype=tf_module.float32)
    return tf_module.reduce_sum(absolute_error) / denominator


@_register_serializable
def detection_class_accuracy(y_true, y_pred):
    """Positive-cell class accuracy using argmax over detector class channels."""
    tf_module = require_tensorflow()
    positive_mask = tf_module.squeeze(y_true[..., 0:1], axis=-1)
    class_true = y_true[..., 5:]
    class_pred = tf_module.nn.sigmoid(y_pred[..., 5:])
    true_index = tf_module.argmax(class_true, axis=-1, output_type=tf_module.int32)
    pred_index = tf_module.argmax(class_pred, axis=-1, output_type=tf_module.int32)
    correct = tf_module.cast(tf_module.equal(true_index, pred_index), tf_module.float32) * positive_mask
    denominator = tf_module.maximum(tf_module.reduce_sum(positive_mask), tf_module.constant(1.0, dtype=tf_module.float32))
    return tf_module.reduce_sum(correct) / denominator


def get_custom_objects() -> dict[str, Any]:
    """Return the custom losses and metrics required when reloading the model."""
    return {
        "detection_loss": detection_loss,
        "detection_objectness_accuracy": detection_objectness_accuracy,
        "detection_bbox_mae": detection_bbox_mae,
        "detection_class_accuracy": detection_class_accuracy,
    }


def build_cnn_model(config: dict[str, Any], train_config: dict[str, Any]):
    """Build and compile the lightweight grid-based thermal detector."""
    tf_module = require_tensorflow()
    input_cfg = config["input"]
    detector_cfg = config["detector"]
    detection_class_count = len([class_name for class_name in config["class_names"] if class_name != "empty"])
    output_channels = 1 + 4 + detection_class_count
    expected_grid_height = int(detector_cfg["grid_height"])
    expected_grid_width = int(detector_cfg["grid_width"])
    learning_rate = float(train_config["learning_rate"])

    inputs = tf_module.keras.Input(
        shape=(int(input_cfg["height"]), int(input_cfg["width"]), int(input_cfg["channels"])),
        name="temp14_input",
    )
    x = tf_module.keras.layers.Conv2D(16, kernel_size=3, padding="same", activation="relu")(inputs)
    x = tf_module.keras.layers.MaxPooling2D(pool_size=2)(x)
    x = tf_module.keras.layers.Conv2D(32, kernel_size=3, padding="same", activation="relu")(x)
    x = tf_module.keras.layers.MaxPooling2D(pool_size=2)(x)
    x = tf_module.keras.layers.Conv2D(64, kernel_size=3, padding="same", activation="relu")(x)
    x = tf_module.keras.layers.MaxPooling2D(pool_size=2)(x)
    x = tf_module.keras.layers.Conv2D(64, kernel_size=3, padding="same", activation="relu")(x)
    detection_output = tf_module.keras.layers.Conv2D(output_channels, kernel_size=1, padding="same", name="detection_head")(x)

    output_shape = detection_output.shape
    if int(output_shape[1]) != expected_grid_height or int(output_shape[2]) != expected_grid_width:
        raise ValueError(
            "detector grid shape does not match model output shape: "
            f"expected {(expected_grid_height, expected_grid_width)}, got {(int(output_shape[1]), int(output_shape[2]))}"
        )

    model = tf_module.keras.Model(
        inputs=inputs,
        outputs=detection_output,
        name="stm32n6_thermal_detector",
    )
    model.compile(
        optimizer=tf_module.keras.optimizers.Adam(learning_rate=learning_rate),
        loss=detection_loss,
        metrics=[
            detection_objectness_accuracy,
            detection_bbox_mae,
            detection_class_accuracy,
        ],
    )
    return model
