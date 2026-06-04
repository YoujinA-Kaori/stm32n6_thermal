#!/usr/bin/env python3
"""Interactive bounding-box annotation tool for thermal .bin frames."""

from __future__ import annotations

import argparse
import tkinter as tk
from pathlib import Path
from tkinter import messagebox
from typing import Any

import _bootstrap

_bootstrap.setup_python_path()

from src.common import (
    AnnotatedObject,
    annotation_path_for_bin,
    get_class_names,
    get_detection_class_names,
    load_dataset_config,
    load_json,
    load_temp14_frame,
    normalize_sidecar_annotation,
    resolve_workspace_path,
    save_json,
    temp14_preview_u8,
)


class AnnotationTool:
    """Interactive Tk application for multi-object bounding-box labeling."""

    def __init__(
        self,
        root: tk.Tk,
        bin_paths: list[Path],
        config: dict[str, Any],
        scale: int,
        raw_root: Path | None,
    ) -> None:
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
        self.detection_class_names = get_detection_class_names(config)
        self.raw_root = raw_root
        self.index = 0
        self.current_primary_class_name: str | None = None
        self.current_objects: list[AnnotatedObject] = []
        self.current_empty = False
        self.selected_object_class = self.detection_class_names[0] if self.detection_class_names else "person"
        self.photo_image = None
        self.preview_image = None
        self.frame_width = int(config["input"]["width"])
        self.frame_height = int(config["input"]["height"])
        self.drag_start_xy: tuple[int, int] | None = None
        self.drag_current_xy: tuple[int, int] | None = None
        self.class_colors = {
            "person": "#ff4d4f",
            "hand": "#fa8c16",
            "hot_object": "#fadb14",
            "circuit_board_normal": "#52c41a",
            "circuit_board_abnormal_hotspot": "#36cfc9",
            "empty": "#8c8c8c",
        }

        self.root.title("Thermal BBox Annotator")
        self.root.geometry(f"{self.frame_width * self.scale + 140}x{self.frame_height * self.scale + 360}")

        self.info_var = tk.StringVar(value="")
        self.status_var = tk.StringVar(value="")
        self.selected_class_var = tk.StringVar(value="")
        self.primary_class_var = tk.StringVar(value="")
        self.object_count_var = tk.StringVar(value="")
        self.object_list_var = tk.StringVar(value="")

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
        self.canvas.bind("<ButtonPress-1>", self._on_left_press)
        self.canvas.bind("<B1-Motion>", self._on_left_drag)
        self.canvas.bind("<ButtonRelease-1>", self._on_left_release)
        self.canvas.bind("<Button-3>", self._on_right_click)

        control_frame = tk.Frame(self.root)
        control_frame.pack(fill=tk.X, padx=8, pady=8)
        tk.Label(control_frame, text="当前框类别", font=("Microsoft YaHei UI", 10)).grid(row=0, column=0, sticky="w")
        tk.Label(control_frame, textvariable=self.selected_class_var, font=("Microsoft YaHei UI", 10, "bold")).grid(row=0, column=1, sticky="w", padx=(4, 12))
        tk.Label(control_frame, text="主类别", font=("Microsoft YaHei UI", 10)).grid(row=0, column=2, sticky="w")
        tk.Label(control_frame, textvariable=self.primary_class_var, font=("Microsoft YaHei UI", 10, "bold")).grid(row=0, column=3, sticky="w", padx=(4, 12))
        tk.Label(control_frame, text="目标数", font=("Microsoft YaHei UI", 10)).grid(row=0, column=4, sticky="w")
        tk.Label(control_frame, textvariable=self.object_count_var, font=("Microsoft YaHei UI", 10, "bold")).grid(row=0, column=5, sticky="w", padx=(4, 12))

        button_frame = tk.Frame(self.root)
        button_frame.pack(fill=tk.X, padx=8)
        buttons = [
            ("保存并下一张", self.save_and_next),
            ("标记为空", self.mark_empty_and_next),
            ("上一张", self.previous_sample),
            ("下一张", self.next_sample),
            ("删除最后框", self.delete_last_object),
            ("清空当前框", self.clear_objects),
            ("退出", self.root.destroy),
        ]
        for index, (label, command) in enumerate(buttons):
            tk.Button(button_frame, text=label, command=command, width=14).grid(row=0, column=index, padx=3, pady=3)

        object_list_frame = tk.Frame(self.root)
        object_list_frame.pack(fill=tk.X, padx=8, pady=8)
        tk.Label(object_list_frame, text="当前框列表:", anchor="w", justify=tk.LEFT, font=("Microsoft YaHei UI", 9, "bold")).pack(fill=tk.X)
        tk.Label(
            object_list_frame,
            textvariable=self.object_list_var,
            anchor="w",
            justify=tk.LEFT,
            font=("Consolas", 9),
            fg="#333333",
        ).pack(fill=tk.X)

        help_lines = [
            "操作说明:",
            "1. 鼠标左键按下并拖拽：绘制当前类别框",
            "2. 鼠标右键：删除点击位置最近的框",
            "3. Enter / S：保存并下一张",
            "4. E：标记为空并保存",
            "5. 数字 1-5：切换目标类别",
            "6. Delete / Backspace：删除最后一个框",
            "7. C：清空当前样本所有框",
            "8. 左 / 右方向键：上一张 / 下一张",
            "9. Q / Esc：退出",
        ]
        tk.Label(
            self.root,
            text="\n".join(help_lines),
            justify=tk.LEFT,
            anchor="w",
            font=("Microsoft YaHei UI", 9),
            fg="#444444",
        ).pack(fill=tk.X, padx=8, pady=8)

    def _bind_keys(self) -> None:
        """Register keyboard shortcuts."""
        self.root.bind("<Return>", lambda _event: self.save_and_next())
        self.root.bind("s", lambda _event: self.save_and_next())
        self.root.bind("S", lambda _event: self.save_and_next())
        self.root.bind("e", lambda _event: self.mark_empty_and_next())
        self.root.bind("E", lambda _event: self.mark_empty_and_next())
        self.root.bind("<Left>", lambda _event: self.previous_sample())
        self.root.bind("<Right>", lambda _event: self.next_sample())
        self.root.bind("<BackSpace>", lambda _event: self.delete_last_object())
        self.root.bind("<Delete>", lambda _event: self.delete_last_object())
        self.root.bind("c", lambda _event: self.clear_objects())
        self.root.bind("C", lambda _event: self.clear_objects())
        self.root.bind("<Escape>", lambda _event: self.root.destroy())
        self.root.bind("q", lambda _event: self.root.destroy())
        self.root.bind("Q", lambda _event: self.root.destroy())

        for index, class_name in enumerate(self.detection_class_names, start=1):
            self.root.bind(str(index), lambda _event, name=class_name: self._set_selected_class(name))

    def _relative_display_path(self, bin_path: Path) -> str:
        """Build a compact display path."""
        if self.raw_root is not None:
            try:
                return str(bin_path.relative_to(self.raw_root))
            except ValueError:
                pass
        return str(bin_path)

    def _derive_primary_class_name(self, bin_path: Path) -> str | None:
        """Infer the primary class from folder structure."""
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

    def _clamp_canvas_xy(self, canvas_x: int, canvas_y: int) -> tuple[int, int]:
        """Clamp one canvas-space point into the valid raw frame range."""
        frame_x = max(0, min(self.frame_width - 1, int(canvas_x / self.scale)))
        frame_y = max(0, min(self.frame_height - 1, int(canvas_y / self.scale)))
        return frame_x, frame_y

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

        annotation = normalize_sidecar_annotation(annotation_payload)
        derived_primary = self._derive_primary_class_name(bin_path)
        self.current_primary_class_name = (
            annotation.primary_class_name
            if annotation is not None and annotation.primary_class_name is not None
            else derived_primary
            if derived_primary is not None
            else "empty"
        )
        self.current_empty = bool(annotation.is_empty) if annotation is not None else self.current_primary_class_name == "empty"
        self.current_objects = list(annotation.objects) if annotation is not None else []
        self.drag_start_xy = None
        self.drag_current_xy = None
        if self.current_primary_class_name == "empty" and not self.current_objects:
            self.current_empty = True
        if self.current_objects and self.selected_object_class not in self.detection_class_names:
            self.selected_object_class = self.current_objects[0].class_name

        self.preview_image = preview_image
        self._refresh_canvas()
        self._update_status("")

    def _refresh_canvas(self) -> None:
        """Redraw the preview image and all bbox overlays."""
        if self.preview_image is None:
            return

        overlay = self.preview_image.copy()
        draw = self.ImageDraw.Draw(overlay)

        for object_index, annotated_object in enumerate(self.current_objects, start=1):
            color = self.class_colors.get(annotated_object.class_name, "#ff4d4f")
            x_min = int(round(annotated_object.x_min * self.scale))
            y_min = int(round(annotated_object.y_min * self.scale))
            x_max = int(round(annotated_object.x_max * self.scale))
            y_max = int(round(annotated_object.y_max * self.scale))
            draw.rectangle((x_min, y_min, x_max, y_max), outline=color, width=max(1, self.scale // 2))
            draw.text((x_min + 2, max(0, y_min - 14)), f"{object_index}:{annotated_object.class_name}", fill=color)

        if self.drag_start_xy is not None and self.drag_current_xy is not None:
            x0, y0 = self.drag_start_xy
            x1, y1 = self.drag_current_xy
            draft_box = (
                min(x0, x1) * self.scale,
                min(y0, y1) * self.scale,
                max(x0, x1) * self.scale,
                max(y0, y1) * self.scale,
            )
            draft_color = self.class_colors.get(self.selected_object_class, "#ff4d4f")
            draw.rectangle(draft_box, outline=draft_color, width=max(1, self.scale // 2))

        self.photo_image = self.ImageTk.PhotoImage(overlay)
        self.canvas.delete("all")
        self.canvas.create_image(0, 0, anchor="nw", image=self.photo_image)

        self.info_var.set(f"[{self.index + 1}/{len(self.bin_paths)}] {self._relative_display_path(self.bin_paths[self.index])}")
        self.selected_class_var.set(self.selected_object_class)
        self.primary_class_var.set(self.current_primary_class_name if self.current_primary_class_name is not None else "(未设置)")
        self.object_count_var.set("empty" if self.current_empty else str(len(self.current_objects)))
        if self.current_objects:
            object_lines = [
                f"{index + 1}. {annotated_object.class_name:<22} [{int(annotated_object.x_min):3d},{int(annotated_object.y_min):3d}] -> [{int(annotated_object.x_max):3d},{int(annotated_object.y_max):3d}]"
                for index, annotated_object in enumerate(self.current_objects)
            ]
            self.object_list_var.set("\n".join(object_lines))
        else:
            self.object_list_var.set("(当前无标注框，或已标记为空)")

    def _update_status(self, message: str) -> None:
        """Update the transient status label."""
        annotation_path = annotation_path_for_bin(self.bin_paths[self.index])
        status = f"JSON: {annotation_path.name}"
        if message:
            status += f" | {message}"
        self.status_var.set(status)

    def _set_selected_class(self, class_name: str) -> None:
        """Switch the currently selected object class."""
        self.selected_object_class = class_name
        self._refresh_canvas()
        self._update_status(f"当前框类别切换为 {class_name}")

    def _on_left_press(self, event: tk.Event) -> None:
        """Start one bbox drag."""
        self.drag_start_xy = self._clamp_canvas_xy(event.x, event.y)
        self.drag_current_xy = self.drag_start_xy
        self._refresh_canvas()

    def _on_left_drag(self, event: tk.Event) -> None:
        """Update the draft bbox while dragging."""
        if self.drag_start_xy is None:
            return
        self.drag_current_xy = self._clamp_canvas_xy(event.x, event.y)
        self._refresh_canvas()

    def _on_left_release(self, event: tk.Event) -> None:
        """Commit one bbox when the drag is released."""
        if self.drag_start_xy is None:
            return

        end_xy = self._clamp_canvas_xy(event.x, event.y)
        start_x, start_y = self.drag_start_xy
        end_x, end_y = end_xy
        x_min = min(start_x, end_x)
        y_min = min(start_y, end_y)
        x_max = max(start_x, end_x) + 1
        y_max = max(start_y, end_y) + 1

        self.drag_start_xy = None
        self.drag_current_xy = None
        if (x_max - x_min) < 2 or (y_max - y_min) < 2:
            self._refresh_canvas()
            self._update_status("框太小，已忽略；请拖出更清晰的 bbox")
            return

        self.current_objects.append(
            AnnotatedObject(
                class_name=self.selected_object_class,
                x_min=float(x_min),
                y_min=float(y_min),
                x_max=float(x_max),
                y_max=float(y_max),
            )
        )
        self.current_empty = False
        if self.current_primary_class_name == "empty":
            derived_primary = self._derive_primary_class_name(self.bin_paths[self.index])
            self.current_primary_class_name = derived_primary if derived_primary is not None else self.selected_object_class
        self._refresh_canvas()
        self._update_status(f"新增 {self.selected_object_class} bbox [{x_min},{y_min}] -> [{x_max},{y_max}]")

    def _on_right_click(self, event: tk.Event) -> None:
        """Remove the nearest annotated box to the clicked point."""
        if not self.current_objects:
            self._update_status("当前没有可删除的框")
            return

        x, y = self._clamp_canvas_xy(event.x, event.y)
        containing_indices = [
            index
            for index, annotated_object in enumerate(self.current_objects)
            if annotated_object.x_min <= x <= annotated_object.x_max and annotated_object.y_min <= y <= annotated_object.y_max
        ]
        if containing_indices:
            remove_index = containing_indices[-1]
        else:
            remove_index = min(
                range(len(self.current_objects)),
                key=lambda index: (self.current_objects[index].center_x - x) ** 2 + (self.current_objects[index].center_y - y) ** 2,
            )

        removed = self.current_objects.pop(remove_index)
        if not self.current_objects:
            self.current_empty = self.current_primary_class_name == "empty"
        self._refresh_canvas()
        self._update_status(
            f"已删除 {removed.class_name} bbox [{int(removed.x_min)},{int(removed.y_min)}] -> [{int(removed.x_max)},{int(removed.y_max)}]"
        )

    def _build_annotation_payload(self) -> dict[str, Any]:
        """Serialize the current UI state into the sidecar JSON format."""
        if self.current_empty or not self.current_objects:
            return {
                "primary_class_name": "empty",
                "empty": True,
                "objects": [],
            }

        primary_class_name = self.current_primary_class_name if self.current_primary_class_name is not None else self.selected_object_class
        return {
            "primary_class_name": primary_class_name,
            "objects": [
                {
                    "class_name": annotated_object.class_name,
                    "x_min": int(annotated_object.x_min),
                    "y_min": int(annotated_object.y_min),
                    "x_max": int(annotated_object.x_max),
                    "y_max": int(annotated_object.y_max),
                }
                for annotated_object in self.current_objects
            ],
        }

    def _save_current_annotation(self) -> bool:
        """Save the current annotation sidecar."""
        bin_path = self.bin_paths[self.index]
        annotation_path = annotation_path_for_bin(bin_path)
        derived_primary = self._derive_primary_class_name(bin_path)

        if self.current_empty and derived_primary is not None and derived_primary != "empty":
            messagebox.showerror(
                "保存失败",
                f"当前样本位于 {derived_primary} 目录下，不能直接保存为 empty。\n"
                "请先把该样本移动到 empty 目录，再做空场景标注。",
            )
            return False

        if not self.current_empty and not self.current_objects:
            messagebox.showerror("保存失败", "当前不是 empty，但没有任何框。请先拖出目标框或按 E 标为空。")
            return False

        if not self.current_empty and derived_primary is not None:
            object_class_names = {annotated_object.class_name for annotated_object in self.current_objects}
            if derived_primary not in object_class_names:
                messagebox.showerror(
                    "保存失败",
                    f"当前样本目录主类是 {derived_primary}，但标注框里没有这个类别。\n"
                    "请确认样本是否放错目录，或者补上主类目标框后再保存。",
                )
                return False

        save_json(annotation_path, self._build_annotation_payload())
        self._update_status("标注已保存")
        return True

    def save_and_next(self) -> None:
        """Save the current annotation and advance to the next sample."""
        if self._save_current_annotation():
            self.next_sample()

    def mark_empty_and_next(self) -> None:
        """Mark the current frame as empty, save, and advance."""
        self.current_empty = True
        self.current_primary_class_name = "empty"
        self.current_objects = []
        self.drag_start_xy = None
        self.drag_current_xy = None
        self._refresh_canvas()
        if self._save_current_annotation():
            self.next_sample()

    def delete_last_object(self) -> None:
        """Delete the last annotated box in the list."""
        if not self.current_objects:
            self._update_status("当前没有可删除的框")
            return
        removed = self.current_objects.pop()
        if not self.current_objects and self.current_primary_class_name == "empty":
            self.current_empty = True
        self._refresh_canvas()
        self._update_status(f"已删除最后一个框 {removed.class_name}")

    def clear_objects(self) -> None:
        """Clear all annotated boxes for the current sample without saving."""
        self.current_objects = []
        self.current_empty = False
        self.drag_start_xy = None
        self.drag_current_xy = None
        self._refresh_canvas()
        self._update_status("当前样本的所有框已清空")

    def next_sample(self) -> None:
        """Advance to the next sample."""
        if self.index >= len(self.bin_paths) - 1:
            self._update_status("已经是最后一张")
            return
        self.index += 1
        self._load_current_sample()

    def previous_sample(self) -> None:
        """Go back to the previous sample."""
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
    parser = argparse.ArgumentParser(description="Interactive bounding-box annotation tool for thermal .bin frames")
    parser.add_argument("--config", type=Path, default=None, help="Path to dataset_config.json")
    parser.add_argument(
        "--input",
        type=Path,
        default=None,
        help="Input .bin file or directory. Defaults to the configured raw dataset root.",
    )
    parser.add_argument("--raw-root", type=Path, default=None, help="Optional raw dataset root for relative display")
    parser.add_argument("--scale", type=int, default=4, help="Preview upscaling factor")
    return parser


def main() -> int:
    """Program entry point."""
    args = build_arg_parser().parse_args()
    config = load_dataset_config(args.config)

    input_path = args.input if args.input is not None else resolve_workspace_path(config["dataset_paths"]["raw_root"])
    raw_root = args.raw_root if args.raw_root is not None else resolve_workspace_path(config["dataset_paths"]["raw_root"])
    bin_paths = collect_bin_paths(input_path)

    root = tk.Tk()
    AnnotationTool(
        root=root,
        bin_paths=bin_paths,
        config=config,
        scale=args.scale,
        raw_root=raw_root if raw_root.exists() else None,
    )
    root.mainloop()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
