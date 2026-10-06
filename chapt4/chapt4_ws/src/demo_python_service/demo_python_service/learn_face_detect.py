import cv2
import face_recognition
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from ament_index_python.packages import get_package_share_directory

FONT_PATH = '/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc'

#ds添加的
def draw_chinese_text(image, text, position, font_size=48):
    # OpenCV 的 putText 不支持中文，借用 PIL 画中文，再转回 OpenCV 图像
    image_pil = Image.fromarray(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(image_pil)
    font = ImageFont.truetype(FONT_PATH, font_size)
    draw.text(position, text, font=font, fill=(255, 0, 0))
    return cv2.cvtColor(np.array(image_pil), cv2.COLOR_RGB2BGR)


def main():
    image_path = get_package_share_directory('demo_python_service') + '/resource/default.jpg'
    image = cv2.imread(image_path)
    if image is None:
        print(f'图片读取失败：{image_path}')
        return

    face_locations = face_recognition.face_locations(
        image, number_of_times_to_upsample=1, model='hog')

    for top, right, bottom, left in face_locations:
        cv2.rectangle(image, (left, top), (right, bottom), (255, 0, 0), 4)

    if not face_locations:
        print('未识别到人脸')
        image = draw_chinese_text(image, '未识别到人脸', (30, 30))

    cv2.imshow('Face Detection Result', image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
