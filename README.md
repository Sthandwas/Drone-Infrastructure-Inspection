# Drone-Based Municipal Infrastructure Inspection

A DJI Tello drone streams video to a laptop, where a YOLOv8n model classifies **streetlights and traffic lights as working or faulty**. A Streamlit dashboard shows the detections and compares them with ground-truth fault states from an Arduino-driven miniature city.

Final-year engineering project, BEngTech Electronic and Computer Engineering, Durban University of Technology (2026).

![Training curves](results.png)

## How it works

1. **Ground truth:** an Arduino Mega (`LED_CONTROL.ino`) drives the miniature city's traffic lights and streetlights, alternating between a working phase and a faulty phase (flickering red light, stuck-dim LEDs, dimmed streetlights) and logging the state.
2. **Capture:** the drone is flown over the model (`FLYING_DRONE.py`) and its video is streamed to the laptop.
3. **Detection:** YOLOv8n classifies four classes: `traffic_light-working`, `traffic_light-faulty`, `streetlight-working`, `streetlight-faulty`.
4. **Dashboard:** a Streamlit app displays detections and compares them with the Arduino log.

## Results (held-out test set)

Evaluated on 431 images (1,724 labelled objects) from recordings kept separate from training. The data was split by video to avoid leakage.

| Metric | Value |
|---|---|
| mAP@0.5 | 0.881 |
| mAP@0.5:0.95 | 0.411 |
| Precision | 0.897 |
| Recall | 0.876 |
| Mean F1 | 0.886 |

| Class | AP@0.5 |
|---|---|
| traffic_light-working | 0.783 |
| traffic_light-faulty | 0.821 |
| streetlight-working | 0.956 |
| streetlight-faulty | 0.966 |

Speed on a Tesla T4 GPU (Google Colab): 2.8 ms preprocessing, 6.3 ms inference, 2.5 ms postprocessing per image.

## Limitations

- Small custom dataset recorded in one indoor miniature environment, so results may not transfer to real streets.
- Traffic lights were harder to classify than streetlights.
- The drone is flown manually, with no autonomous navigation.
- Speed was measured on a GPU. Laptop timings will differ.

## Tech

Python · YOLOv8 (Ultralytics) · Streamlit · Arduino · DJI Tello · Google Colab

## Files

| File | Purpose |
|---|---|
| `FLYING_DRONE.py` | Drone control and video capture |
| `LED_CONTROL.ino` | Arduino sketch for the fault-simulation rig |
| `results.png` | Training curves |
