#!/usr/bin/env python3
"""Model definition for the lightweight STM32N6 thermal classifier."""

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


def build_cnn_model(config: dict[str, Any], train_config: dict[str, Any]):
    """Build and compile the lightweight multi-task thermal network."""
    tf_module = require_tensorflow()
    input_cfg = config["input"]
    num_classes = len(config["class_names"])
    learning_rate = float(train_config["learning_rate"])
    heatmap_loss_weight = float(train_config.get("heatmap_loss_weight", 1.0))

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

    heat_x = tf_module.keras.layers.Conv2D(32, kernel_size=3, padding="same", activation="relu")(x)
    heat_x = tf_module.keras.layers.UpSampling2D(size=2)(heat_x)
    heat_x = tf_module.keras.layers.Conv2D(16, kernel_size=3, padding="same", activation="relu")(heat_x)
    heat_x = tf_module.keras.layers.UpSampling2D(size=2)(heat_x)
    heat_x = tf_module.keras.layers.Conv2D(8, kernel_size=3, padding="same", activation="relu")(heat_x)
    heat_x = tf_module.keras.layers.UpSampling2D(size=2)(heat_x)
    heatmap_output = tf_module.keras.layers.Conv2D(1, kernel_size=1, padding="same", activation="sigmoid", name="heatmap")(heat_x)

    x = tf_module.keras.layers.GlobalAveragePooling2D()(x)
    x = tf_module.keras.layers.Dense(64, activation="relu")(x)
    class_output = tf_module.keras.layers.Dense(num_classes, activation="softmax", name="class_probs")(x)

    model = tf_module.keras.Model(
        inputs=inputs,
        outputs=[class_output, heatmap_output],
        name="stm32n6_thermal_multitask",
    )
    model.compile(
        optimizer=tf_module.keras.optimizers.Adam(learning_rate=learning_rate),
        loss={
            "class_probs": "sparse_categorical_crossentropy",
            "heatmap": "mse",
        },
        loss_weights={
            "class_probs": 1.0,
            "heatmap": heatmap_loss_weight,
        },
        metrics={
            "class_probs": ["accuracy"],
            "heatmap": ["mae"],
        },
    )
    return model
