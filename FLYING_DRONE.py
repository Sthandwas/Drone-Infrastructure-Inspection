"""
Flying / Video Capture Module - DJI Tello
-------------------------------------------
Connects to the Tello, provides manual keyboard flight control, and pulls
the live video feed for display (and later, hand-off to the YOLO module).

SETUP BEFORE RUNNING:
  1. Turn the Tello on.
  2. On your laptop, connect to the Tello's own Wi-Fi network (it appears
     as "TELLO-XXXXXX" in your Wi-Fi list) -- do this the same way you'd
     join any Wi-Fi network, BEFORE running this script.
  3. pip install djitellopy opencv-python

CONTROLS (with the video window focused):
  T = Takeoff        L = Land
  W/A/S/D = Move forward/left/back/rightt
  R/F = Move up/down
  Q/E = Rotate counter-clockwise / clockwise
  ESC = Land (if flying) and quit safely

NOTE: Send movement commands with a brief pause between them -- sending
commands too rapidly can cause the Tello to reject them ("error Not
joystick"). This script's loop is already paced by cv2.waitKey(1), which
is generally enough, but avoid holding a key down continuously.
"""

import cv2
from djitellopy import Tello

MOVE_DISTANCE_CM = 30   # must be between 20-500 per the Tello SDK
ROTATE_DEGREES = 30     # must be between 1-360 per the Tello SDK

def main():
    tello = Tello()
    tello.connect()
    print(f"Connected. Battery: {tello.get_battery()}%")

    tello.streamon()
    frame_reader = tello.get_frame_read()

    is_flying = False
    print("Controls: T=Takeoff  L=Land  W/A/S/D=Move  R/F=Up/Down  "
          "Q/E=Rotate  ESC=Quit")

    try:
        while True:
            frame = frame_reader.frame  # latest video frame -- this is what
            if frame is None:            # gets handed to the YOLO module next
                continue

            display_frame = cv2.resize(frame, (720, 480))
            cv2.imshow("Tello Live Feed",
                       cv2.cvtColor(display_frame, cv2.COLOR_RGB2BGR))

            key = cv2.waitKey(1) & 0xFF

            if key == ord('t') and not is_flying:
                tello.takeoff()
                is_flying = True
            elif key == ord('l') and is_flying:
                tello.land()
                is_flying = False
            elif key == ord('w'):
                tello.move_forward(MOVE_DISTANCE_CM)
            elif key == ord('s'):
                tello.move_back(MOVE_DISTANCE_CM)
            elif key == ord('a'):
                tello.move_left(MOVE_DISTANCE_CM)
            elif key == ord('d'):
                tello.move_right(MOVE_DISTANCE_CM)
            elif key == ord('r'):
                tello.move_up(MOVE_DISTANCE_CM)
            elif key == ord('f'):
                tello.move_down(MOVE_DISTANCE_CM)
            elif key == ord('q'):
                tello.rotate_counter_clockwise(ROTATE_DEGREES)
            elif key == ord('e'):
                tello.rotate_clockwise(ROTATE_DEGREES)
            elif key == 27:  # ESC
                break

    finally:
        if is_flying:
            tello.land()
        tello.streamoff()
        cv2.destroyAllWindows()
        print("Session ended safely.")


if __name__ == "__main__":
    main()
