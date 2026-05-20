#!/usr/bin/env python3
"""Interactive center-point annotation tool for thermal .bin frames."""

from __future__ import annotations

import argparse
import tkinter as tk
from pathlib import Path
from tkinter import messagebox
from typing import Any

import _bootstrap

_bootstrap.setup_python_path()

from src.common import (
    annotation_path_for_bin,
    get_class_names,
    load_dataset_config,
    load_json,
    load_temp14_frame,
    normalize_sidecar_annotation,
    resolve_workspace_path,
    save_json,
    temp14_preview_u8,
)


class AnnotationTool:
    """Interactive Tk application for center-point labeling."""

    def __init__(self, root: tk.Tk, bin_paths: list[Path], config: dict[str, Any], scale: int, sigma_px: float | None, raw_root: Path | None) -> None:
        try:
            from PIL import Image, ImageDraw, ImageTk
        except ImportError as exc:  # pragma: no cover
            raise SystemExit(f"Pillow is required for the annotation tool: {exc}") from exc

        self.Image = Image
        self.ImageDraw = ImageDraw
        self.ImageTk = ImageTk
        self.root = root
        self.bin_paths = bin_paths
        self.config = config
        self.scale = max(int(scale), 1)
        self.class_names = get_class_names(config)
        self.default_sigma_px = float(sigma_px if sigma_px is not None else config["supervision"]["heatmap_sigma_px"])
        self.raw_root = raw_root
        self.index = 0
        self.current_center: tuple[int, int] | None = None
        self.current_class_name: str | None = None
        self.current_empty = False
        self.photo_image = None
        self.preview_image = None
        self.frame_width = int(config["input"]["width"])
        self.frame_height = int(config["input"]["height"])

        self.root.title("Thermal Center-Point Annotator")
        self.root.geometry(f"{self.frame_width * self.scale + 80}x{self.frame_height * self.scale + 220}")

        self.info_var = tk.StringVar(value="")
        self.status_var = tk.StringVar(value="")
        self.class_var = tk.StringVar(value="")
        self.center_var = tk.StringVar(value="")

        self._build_ui()
        self._bind_keys()
        self._load_current_sample()

    def _build_ui(self) -> None:
        """Construct the main window."""
        top_frame = tk.Frame(self.root)
        top_frame.pack(fill=tk.X, padx=8, pady=8)

        tk.Label(top_frame, textvariable=self.info_var, anchor="w", justify=tk.LEFT, font=("Microsoft YaHei UI", 10)).pack(fill=tk.X)
        tk.Label(top_frame, textvariable=self.status_var, anchor="w", justify=tk.LEFT, fg="#0B5", font=("Microsoft YaHei UI", 10)).pack(fill=tk.X)

        preview_frame = tk.Frame(self.root)
        preview_frame.pack(fill=tk.BOTH, expand=False, padx=8)
        self.canvas = tk.Canvas(
            preview_frame,
            width=self.frame_width * self.scale,
            height=self.frame_height * self.scale,
            bg="black",
            highlightthickness=1,
            highlightbackground="#666666",
        )
        self.canvas.pack()
        self.canvas.bind("<Button-1>", self._on_left_click)

        control_frame = tk.Frame(self.root)
        control_frame.pack(fill=tk.X, padx=8, pady=8)

        tk.Label(control_frame, text="类别:", font=("Microsoft YaHei UI", 10)).grid(row=0, column=0, sticky="w")
        tk.Label(control_frame, textvariable=self.class_var, font=("Microsoft YaHei UI", 10, "bold")).grid(row=0, column=1, sticky="w", padx=(4, 12))
        tk.Label(control_frame, text="中心点:", font=("Microsoft YaHei UI", 10)).grid(row=0, column=2, sticky="w")
        tk.Label(control_frame, textvariable=self.center_var, font=("Microsoft YaHei UI", 10, "bold")).grid(row=0, column=3, sticky="w", padx=(4, 12))

        button_frame = tk.Frame(self.root)
        button_frame.pack(fill=tk.X, padx=8)

        buttons = [
            ("保存并下一张", self.save_and_next),
            ("标记为空", self.mark_empty_and_next),
            ("上一张", self.previous_sample),
            ("下一张", self.next_sample),
            ("删除标注", self.delete_annotation),
            ("退出", self.root.destroy),
        ]
        for index, (label, command) in enumerate(buttons):
            tk.Button(button_frame, text=label, command=command, width=14).grid(row=0, column=index, padx=3, pady=3)

        help_text = (
            "操作说明:\n"
            "1. 鼠标左键点击图像设置中心点\n"
            "2. Enter / S: 保存并下一张\n"
            "3. E: 标记为空并保存\n"
            "4. 左/右方向键: 上一张/下一张\n"
            "5. Delete / Backspace: 删除当前 JSON 标注\n"
            "6. 数字 1-5: 快速切换类别\n"
            "7. Q / Esc: 退出"
        )
        tk.Label(self.root, text=help_text, justify=tk.LEFT, anchor="w", font=("Microsoft YaHei UI", 9), fg="#444444").pack(fill=tk.X, padx=8, pady=8)

    def _bind_keys(self) -> None:
        """Register keyboard shortcuts."""
        self.root.bind("<Return>", lambda _event: self.save_and_next())
        self.root.bind("s", lambda _event: self.save_and_next())
        self.root.bind("S", lambda _event: self.save_and_next())
        self.root.bind("e", lambda _event: self.mark_empty_and_next())
        self.root.bind("E", lambda _event: self.mark_empty_and_next())
        self.root.bind("<Left>", lambda _event: self.previous_sample())
        self.root.bind("<Right>", lambda _event: self.next_sample())
        self.root.bind("<BackSpace>", lambda _event: self.delete_annotation())
        self.root.bind("<Delete>", lambda _event: self.delete_annotation())
        self.root.bind("<Escape>", lambda _event: self.root.destroy())
        self.root.bind("q", lambda _event: self.root.destroy())
        self.root.bind("Q", lambda _event: self.root.destroy())

        for index, class_name in enumerate(self.class_names, start=1):
            key = str(index)
            self.root.bind(key, lambda _event, name=class_name: self._set_class_name(name))

    def _relative_display_path(self, bin_path: Path) -> str:
        """Build a compact display path."""
        if self.raw_root is not None:
            try:
                return str(bin_path.relative_to(self.raw_root))
            except ValueError:
                pass
        return str(bin_path)

    def _derive_class_name(self, bin_path: Path) -> str | None:
        """Infer the class name from the folder structure."""
        if self.raw_root is not None:
            try:
                relative = bin_path.relative_to(self.raw_root)
                if relative.parts:
                    first_part = relative.parts[0]
                    if first_part in self.class_names:
                        return first_part
            except ValueError:
                pass

        for parent in bin_path.parents:
            if parent.name in self.class_names:
                return parent.name
        return None

    def _load_current_sample(self) -> None:
        """Load the current sample into the UI."""
        if not self.bin_paths:
            raise SystemExit("no .bin files available for annotation")

        bin_path = self.bin_paths[self.index]
        frame_u16 = load_temp14_frame(bin_path, self.config)
        preview_u8 = temp14_preview_u8(frame_u16, self.config)
        preview_image = self.Image.fromarray(preview_u8, mode="L").convert("RGB")
        preview_image = preview_image.resize((self.frame_width * self.scale, self.frame_height * self.scale), self.Image.NEAREST)

        annotation_payload = None
        annotation_path = annotation_path_for_bin(bin_path)
        if annotation_path.exists():
            annotation_payload = load_json(annotation_path)

        annotation = normalize_sidecar_annotation(annotation_payload, self.default_sigma_px)
        derived_class_name = self._derive_class_name(bin_path)
        self.current_class_name = (
            annotation.class_name
            if annotation is not None and annotation.class_name is not None
            else derived_class_name
            if derived_class_name is not None
            else self.class_names[0]
        )
        self.current_empty = bool(annotation.is_empty) if annotation is not None else self.current_class_name == "empty"
        if annotation is not None and annotation.center_x is not None and annotation.center_y is not None:
            self.current_center = (int(round(annotation.center_x)), int(round(annotation.center_y)))
        else:
            self.current_center = None

        self.preview_image = preview_image
        self._refresh_canvas()
        self._update_status("")

    def _refresh_canvas(self) -> None:
        """Redraw the preview image and overlay."""
        if self.preview_image is None:
            return

        overlay = self.preview_image.copy()
        draw = self.ImageDraw.Draw(overlay)

        if self.current_center is not None:
            center_x, center_y = self.current_center
            scaled_x = center_x * self.scale
            scaled_y = center_y * self.scale
            radius = max(4, self.scale * 2)
            draw.ellipse(
                (scaled_x - radius, scaled_y - radius, scaled_x + radius, scaled_y + radius),
                outline=(255, 64, 64),
                width=max(1, self.scale // 2),
            )
            draw.line((scaled_x - radius * 2, scaled_y, scaled_x + radius * 2, scaled_y), fill=(255, 64, 64), width=max(1, self.scale // 2))
            draw.line((scaled_x, scaled_y - radius * 2, scaled_x, scaled_y + radius * 2), fill=(255, 64, 64), width=max(1, self.scale // 2))

        self.photo_image = self.ImageTk.PhotoImage(overlay)
        self.canvas.delete("all")
        self.canvas.create_image(0, 0, anchor="nw", image=self.photo_image)

        path_display = self._relative_display_path(self.bin_paths[self.index])
        info_text = f"[{self.index + 1}/{len(self.bin_paths)}] {path_display}"
        self.info_var.set(info_text)
        self.class_var.set(self.current_class_name if self.current_class_name is not None else "(未设置)")
        if self.current_empty:
            self.center_var.set("empty")
        elif self.current_center is not None:
            self.center_var.set(f"({self.current_center[0]}, {self.current_center[1]})")
        else:
            self.center_var.set("(未标注)")

    def _update_status(self, message: str) -> None:
        """Update the transient status label."""
        bin_path = self.bin_paths[self.index]
        annotation_path = annotation_path_for_bin(bin_path)
        status = f"JSON: {annotation_path.name}"
        if message:
            status += f" | {message}"
        self.status_var.set(status)

    def _set_class_name(self, class_name: str) -> None:
        """Set the active class name."""
        self.current_class_name = class_name
        self.current_empty = class_name == "empty"
        if self.current_empty:
            self.current_center = None
        self._refresh_canvas()
        self._update_status(f"当前类别切换为 {class_name}")

    def _on_left_click(self, event: tk.Event) -> None:
        """Handle mouse clicks on the preview canvas."""
        x = max(0, min(self.frame_width - 1, int(event.x / self.scale)))
        y = max(0, min(self.frame_height - 1, int(event.y / self.scale)))
        self.current_center = (x, y)
        if self.current_class_name == "empty":
            self.current_class_name = self._derive_class_name(self.bin_paths[self.index]) or "person"
        self.current_empty = False
        self._refresh_canvas()
        self._update_status(f"中心点已设置为 ({x}, {y})")

    def _save_current_annotation(self) -> bool:
        """Save the current annotation sidecar."""
        bin_path = self.bin_paths[self.index]
        annotation_path = annotation_path_for_bin(bin_path)

        if self.current_class_name is None:
            messagebox.showerror("保存失败", "当前类别为空，请先设置类别。")
            return False

        if self.current_class_name != "empty" and self.current_center is None:
            messagebox.showerror("保存失败", "非空样本必须先点击图像设置中心点。")
            return False

        if self.current_empty or self.current_class_name == "empty":
            payload = {
                "class_name": "empty",
                "empty": True,
            }
        else:
            payload = {
                "class_name": self.current_class_name,
                "center_x": int(self.current_center[0]),
                "center_y": int(self.current_center[1]),
                "heatmap_sigma_px": float(self.default_sigma_px),
            }

        save_json(annotation_path, payload)
        self._update_status("标注已保存")
        return True

    def save_and_next(self) -> None:
        """Save the current annotation and advance to the next sample."""
        if self._save_current_annotation():
            self.next_sample()

    def mark_empty_and_next(self) -> None:
        """Mark the current frame as empty, save, and advance."""
        self.current_class_name = "empty"
        self.current_empty = True
        self.current_center = None
        self._refresh_canvas()
        if self._save_current_annotation():
            self.next_sample()

    def delete_annotation(self) -> None:
        """Delete the current sidecar JSON annotation."""
        bin_path = self.bin_paths[self.index]
        annotation_path = annotation_path_for_bin(bin_path)
        if annotation_path.exists():
            annotation_path.unlink()
            self._update_status("当前 JSON 标注已删除")
        else:
            self._update_status("当前样本没有 JSON 标注")
        self.current_center = None
        self.current_empty = False
        self.current_class_name = self._derive_class_name(bin_path) or self.class_names[0]
        self._refresh_canvas()

    def next_sample(self) -> None:
        """Advance to the next sample if possible."""
        if self.index >= len(self.bin_paths) - 1:
            self._update_status("已经是最后一张")
            return
        self.index += 1
        self._load_current_sample()

    def previous_sample(self) -> None:
        """Go back to the previous sample if possible."""
        if self.index <= 0:
            self._update_status("已经是第一张")
            return
        self.index -= 1
        self._load_current_sample()


def collect_bin_paths(input_path: Path) -> list[Path]:
    """Collect .bin files from a file or directory input."""
    if input_path.is_file():
        if input_path.suffix.lower() != ".bin":
            raise SystemExit(f"input file is not a .bin: {input_path}")
        return [input_path]

    if input_path.is_dir():
        bin_paths = sorted(path for path in input_path.rglob("*.bin") if path.is_file())
        if not bin_paths:
            raise SystemExit(f"no .bin files found under: {input_path}")
        return bin_paths

    raise SystemExit(f"input path does not exist: {input_path}")


def build_arg_parser() -> argparse.ArgumentParser:
    """Build the CLI argument parser."""
    parser = argparse.ArgumentParser(description="Interactive center-point annotation tool for thermal .bin frames")
    parser.add_argument("--config", type=Path, default=None, help="Path to dataset_config.json")
    parser.add_argument(
        "--input",
        type=Path,
        default=None,
        help="Input .bin file or directory. Defaults to the configured raw dataset root.",
    )
    parser.add_argument("--raw-root", type=Path, default=None, help="Optional raw dataset root for relative display")
    parser.add_argument("--scale", type=int, default=4, help="Preview upscaling factor")
    parser.add_argument("--sigma-px", type=float, default=None, help="Override heatmap sigma in pixels")
    return parser


def main() -> int:
    """Program entry point."""
    args = build_arg_parser().parse_args()
    config = load_dataset_config(args.config)

    input_path = (
        args.input
        if args.input is not None
        else resolve_workspace_path(config["dataset_paths"]["raw_root"])
    )
    raw_root = args.raw_root if args.raw_root is not None else resolve_workspace_path(config["dataset_paths"]["raw_root"])
    bin_paths = collect_bin_paths(input_path)

    root = tk.Tk()
    app = AnnotationTool(
        root=root,
        bin_paths=bin_paths,
        config=config,
        scale=args.scale,
        sigma_px=args.sigma_px,
        raw_root=raw_root if raw_root.exists() else None,
    )
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
