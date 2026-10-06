#!/usr/bin/env python3
"""从 face_source.jpg 生成 turtlesim 用的 45x45 贴图 src/turtlesim/images/face.png。

改完参数后重新运行本脚本，再执行：
    colcon build --packages-select turtlesim
"""

from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

HERE = Path(__file__).resolve().parent
SOURCE = HERE / "face_source.jpg"
TARGET = HERE / "src" / "turtlesim" / "images" / "face.png"

# 裁剪区域 (left, top, right, bottom)，原图 960x1280。调这四个数可以改变取景
CROP = (30, 20, 910, 900)

# 和原版小海龟贴图保持一致，不要改
SIZE = 45

# "ellipse" 圆形贴纸 / "rounded" 圆角方形 / "square" 方形
MASK = "ellipse"
CORNER = 0.30            # MASK == "rounded" 时的圆角比例
OUTLINE_WIDTH = 0.033    # 贴纸描边宽度（相对贴图尺寸），0 表示不描边
OUTLINE_COLOR = (35, 35, 45, 255)
SUPERSAMPLE = 6          # 描边/遮罩的抗锯齿倍率


def build_mask(size: int, supersample: int) -> Image.Image:
    n = size * supersample
    mask = Image.new("L", (n, n), 0)
    draw = ImageDraw.Draw(mask)
    if MASK == "ellipse":
        draw.ellipse([0, 0, n - 1, n - 1], fill=255)
    elif MASK == "rounded":
        draw.rounded_rectangle([0, 0, n - 1, n - 1], radius=int(n * CORNER), fill=255)
    else:
        draw.rectangle([0, 0, n - 1, n - 1], fill=255)
    return mask.resize((size, size), Image.LANCZOS)


def main() -> None:
    side = CROP[2] - CROP[0]
    img = Image.open(SOURCE).convert("RGB").crop(CROP).resize((side, side), Image.LANCZOS)
    img = img.filter(ImageFilter.UnsharpMask(radius=3, percent=55, threshold=3))
    img = img.convert("RGBA")
    img.putalpha(build_mask(side, SUPERSAMPLE))

    if OUTLINE_WIDTH > 0:
        n = side * SUPERSAMPLE
        width = max(1, round(n * OUTLINE_WIDTH))
        canvas = Image.new("RGBA", (n, n), (0, 0, 0, 0))
        canvas.paste(img.resize((n, n), Image.LANCZOS), (0, 0))
        draw = ImageDraw.Draw(canvas)
        draw.ellipse([0, 0, n - 1, n - 1], outline=OUTLINE_COLOR, width=width)
        img = canvas.resize((side, side), Image.LANCZOS)

    TARGET.parent.mkdir(parents=True, exist_ok=True)
    img.resize((SIZE, SIZE), Image.LANCZOS).save(TARGET)
    print(f"wrote {TARGET} ({SIZE}x{SIZE})")


if __name__ == "__main__":
    main()
