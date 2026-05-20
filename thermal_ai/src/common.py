#!/usr/bin/env python3
"""Shared helpers for the STM32N6 thermal five-class AI workflow."""

from __future__ import annotations

import csv
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

import numpy as np


THERMAL_AI_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = THERMAL_AI_ROOT.parent
DEFAULT_DATASET_CONFIG_PATH = THERMAL_AI_ROOT / "configs" / "dataset_config.json"
DEFAULT_TRAIN_CONFIG_PATH = THERMAL_AI_ROOT / "configs" / "train_config.json"


@dataclass(frozen=True)
class SampleRecord:
    """Description of one thermal frame sample."""

    split: str
    class_name: str
    label_index: int
    group_key: str
    bin_path: Path
    annotation_path: Path | None = None


@dataclass(frozen=True)
class HeatmapAnnotation:
    """Normalized annotation for one thermal frame."""

    class_name: str | None
    center_x: float | None
    center_y: float | None
    sigma_px: float
    is_empty: bool


def load_json(json_path: Path) -> dict[str, Any]:
    """Load a JSON file into a dictionary."""
    with json_path.open("r", encoding="utf-8") as fp:
        return json.load(fp)


def save_json(json_path: Path, payload: dict[str, Any] | list[Any]) -> None:
    """Save a JSON-compatible object using UTF-8 and stable indentation."""
    json_path.parent.mkdir(parents=True, exist_ok=True)
    with json_path.open("w", encoding="utf-8") as fp:
        json.dump(payload, fp, indent=2, ensure_ascii=False)
        fp.write("\n")


def load_dataset_config(config_path: Path | None = None) -> dict[str, Any]:
    """Load the dataset configuration file."""
    path = config_path if config_path is not None else DEFAULT_DATASET_CONFIG_PATH
    return load_json(path)


def load_train_config(config_path: Path | None = None) -> dict[str, Any]:
    """Load the training configuration file."""
    path = config_path if config_path is not None else DEFAULT_TRAIN_CONFIG_PATH
    return load_json(path)


def ensure_dir(dir_path: Path) -> Path:
    """Create a directory if it does not exist."""
    dir_path.mkdir(parents=True, exist_ok=True)
    return dir_path


def resolve_workspace_path(path_value: str | Path) -> Path:
    """Resolve a workspace-relative config path into an absolute path."""
    candidate = Path(path_value)
    if candidate.is_absolute():
        return candidate
    return WORKSPACE_ROOT / candidate


def annotation_path_for_bin(bin_path: Path) -> Path:
    """Return the sidecar JSON path for one thermal frame."""
    return bin_path.with_suffix(".json")


def load_sidecar_annotation(bin_path: Path) -> dict[str, Any] | None:
    """Load an optional per-frame JSON annotation sidecar."""
    annotation_path = annotation_path_for_bin(bin_path)
    if not annotation_path.exists():
        return None
    return load_json(annotation_path)


def normalize_sidecar_annotation(
    annotation_payload: dict[str, Any] | None,
    default_sigma_px: float,
) -> HeatmapAnnotation | None:
    """Normalize a raw sidecar annotation payload into a typed record."""
    if annotation_payload is None:
        return None

    class_name = annotation_payload.get("class_name")
    is_empty = bool(annotation_payload.get("empty", False)) or class_name == "empty"
    center_x = annotation_payload.get("center_x", annotation_payload.get("x"))
    center_y = annotation_payload.get("center_y", annotation_payload.get("y"))
    sigma_px = float(annotation_payload.get("heatmap_sigma_px", annotation_payload.get("sigma_px", default_sigma_px)))

    if center_x is None or center_y is None:
        if is_empty:
            return HeatmapAnnotation(
                class_name=str(class_name) if class_name is not None else None,
                center_x=None,
                center_y=None,
                sigma_px=sigma_px,
                is_empty=True,
            )
        return None

    return HeatmapAnnotation(
        class_name=str(class_name) if class_name is not None else None,
        center_x=float(center_x),
        center_y=float(center_y),
        sigma_px=sigma_px,
        is_empty=is_empty,
    )


def get_class_names(config: dict[str, Any]) -> list[str]:
    """Return the ordered class-name list."""
    return list(config["class_names"])


def get_input_shape(config: dict[str, Any]) -> tuple[int, int, int]:
    """Return the TensorFlow NHWC input shape."""
    input_cfg = config["input"]
    return int(input_cfg["height"]), int(input_cfg["width"]), int(input_cfg["channels"])


def get_frame_shape(config: dict[str, Any]) -> tuple[int, int]:
    """Return the thermal frame height and width."""
    height, width, _ = get_input_shape(config)
    return height, width


def expected_frame_bytes(config: dict[str, Any]) -> int:
    """Return the expected payload size for one .bin temperature frame."""
    return int(config["raw_format"]["frame_bytes"])


def temp14_to_celsius(frame_u16: np.ndarray) -> np.ndarray:
    """Convert temp14 data from kelvin*16 to Celsius."""
    return frame_u16.astype(np.float32) / 16.0 - 273.15


def load_temp14_frame(bin_path: Path, config: dict[str, Any]) -> np.ndarray:
    """Load one raw .bin frame as a 2D uint16 array."""
    height, width = get_frame_shape(config)
    expected_bytes = expected_frame_bytes(config)
    actual_bytes = bin_path.stat().st_size
    if actual_bytes != expected_bytes:
        raise ValueError(
            f"{bin_path} has {actual_bytes} bytes, expected {expected_bytes} bytes "
            f"for {width}x{height} uint16 thermal data"
        )

    frame = np.fromfile(bin_path, dtype="<u2")
    return frame.reshape(height, width)


def normalize_temp14_frame(
    frame_u16: np.ndarray,
    config: dict[str, Any],
    add_channel_axis: bool = True,
) -> np.ndarray:
    """Convert raw temp14 pixels to the fixed normalized float32 input tensor."""
    norm_cfg = config["normalization"]
    temp_c = temp14_to_celsius(frame_u16)
    temp_c_min = float(norm_cfg["temp_c_min"])
    temp_c_max = float(norm_cfg["temp_c_max"])
    scale = temp_c_max - temp_c_min
    if scale <= 0.0:
        raise ValueError("normalization.temp_c_max must be greater than normalization.temp_c_min")

    normalized = np.clip((temp_c - temp_c_min) / scale, 0.0, 1.0).astype(np.float32)
    if add_channel_axis:
        normalized = normalized[..., np.newaxis]
    return normalized


def temp14_preview_u8(frame_u16: np.ndarray, config: dict[str, Any]) -> np.ndarray:
    """Create an 8-bit grayscale preview using the same fixed normalization."""
    normalized = normalize_temp14_frame(frame_u16, config, add_channel_axis=False)
    return np.clip(np.round(normalized * 255.0), 0, 255).astype(np.uint8)


def build_gaussian_heatmap(
    height: int,
    width: int,
    center_x: float | None,
    center_y: float | None,
    sigma_px: float,
) -> np.ndarray:
    """Build a normalized 2D Gaussian heatmap for one frame."""
    if center_x is None or center_y is None:
        return np.zeros((height, width), dtype=np.float32)

    sigma_px = max(float(sigma_px), 1.0)
    y_coords, x_coords = np.mgrid[0:height, 0:width]
    squared_distance = (x_coords - float(center_x)) ** 2 + (y_coords - float(center_y)) ** 2
    heatmap = np.exp(-squared_distance / (2.0 * sigma_px * sigma_px)).astype(np.float32)
    peak = float(np.max(heatmap))
    if peak > 0.0:
        heatmap /= peak
    return heatmap


def iter_bin_files(root_dir: Path) -> list[Path]:
    """Collect .bin files recursively in stable order."""
    return sorted(path for path in root_dir.rglob("*.bin") if path.is_file())


def strip_numeric_suffix(stem: str) -> str:
    """Strip a trailing numeric frame suffix from a filename stem."""
    stripped = re.sub(r"(?:[_-]frame)?[_-]?\d+$", "", stem, flags=re.IGNORECASE)
    stripped = stripped.rstrip("_-")
    return stripped or stem


def derive_group_key(bin_path: Path, class_root: Path) -> str:
    """Derive the session/group key for leakage-safe dataset splitting."""
    relative_path = bin_path.relative_to(class_root)
    if len(relative_path.parts) > 1:
        parent_key = "__".join(relative_path.parts[:-1]).replace(" ", "_")
        return parent_key

    return strip_numeric_suffix(relative_path.stem)


def scan_processed_split(
    processed_root: Path,
    split_name: str,
    class_names: list[str],
) -> list[SampleRecord]:
    """Scan one processed split into structured sample records."""
    samples: list[SampleRecord] = []
    split_root = processed_root / split_name

    for label_index, class_name in enumerate(class_names):
        class_root = split_root / class_name
        if not class_root.exists():
            continue

        for bin_path in iter_bin_files(class_root):
            relative_parent = bin_path.parent.relative_to(class_root)
            if str(relative_parent) == ".":
                group_key = strip_numeric_suffix(bin_path.stem)
            else:
                group_key = relative_parent.as_posix()

            sidecar_path = annotation_path_for_bin(bin_path)

            samples.append(
                SampleRecord(
                    split=split_name,
                    class_name=class_name,
                    label_index=label_index,
                    group_key=group_key,
                    bin_path=bin_path,
                    annotation_path=sidecar_path if sidecar_path.exists() else None,
                )
            )

    return samples


def write_csv(csv_path: Path, fieldnames: list[str], rows: Iterable[dict[str, Any]]) -> None:
    """Write a CSV file using the provided field order."""
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    with csv_path.open("w", encoding="utf-8", newline="") as fp:
        writer = csv.DictWriter(fp, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)


def to_python_scalar(value: Any) -> Any:
    """Convert NumPy scalar types into JSON-safe Python scalars."""
    if isinstance(value, np.generic):
        return value.item()
    return value


def make_json_safe(payload: Any) -> Any:
    """Recursively convert NumPy values inside a nested structure."""
    if isinstance(payload, dict):
        return {str(key): make_json_safe(value) for key, value in payload.items()}
    if isinstance(payload, list):
        return [make_json_safe(value) for value in payload]
    if isinstance(payload, tuple):
        return [make_json_safe(value) for value in payload]
    return to_python_scalar(payload)
