import time
import streamlit as st
import pandas as pd
import os

# --- CONFIGURE CORE WEB INTERFACE ---
st.set_page_config(page_title="Infrastructure Monitor", layout="wide")
st.title("Municipal Infrastructure Inspection Dashboard")

CSV_LOG_PATH = r"C:\Users\Lelo Linda\OneDrive\Desktop\Streamlit_codes\detections_log.csv"
REFRESH_S = 2          # the page reads the log again every 2 seconds

df = None
if os.path.exists(CSV_LOG_PATH):
    try:
        df = pd.read_csv(CSV_LOG_PATH)          # the file may be half-written at this instant: just try again next time
    except Exception:
        df = None

if df is None:
    st.error("Data interface file 'detections_log.csv' not found (or being written). Start live_detect.py first.")
elif df.empty:
    st.info("The detection log is empty. It fills up once the first label has been confirmed over 5 frames.")
else:
    # 1. SYSTEM METRIC BLOCKS
    total_detections = len(df)
    fault_detections = len(df[df["condition"] == "faulty"])
    latest = df.groupby("object_type").tail(1).set_index("object_type")

    c1, c2, c3, c4 = st.columns(4)
    c1.metric("Total Detections Logged", total_detections)
    c2.metric("Faulty Conditions Flagged", fault_detections)
    for col, name, title in ((c3, "traffic_light", "Traffic light (latest)"),
                             (c4, "streetlight", "Streetlight (latest)")):
        if name in latest.index:
            col.metric(title, str(latest.loc[name, "condition"]).upper(),
                       "confidence %.2f at %s" % (latest.loc[name, "confidence"], latest.loc[name, "timestamp"]),
                       delta_color="off")
        else:
            col.metric(title, "no data yet")

    # 2. NEWEST DETECTIONS FIRST
    st.subheader("Recent System Detections Log")
    st.dataframe(df.tail(10).iloc[::-1], use_container_width=True)

    # 3. CHART
    st.subheader("Detections by Infrastructure Condition")
    st.bar_chart(df.groupby(["object_type", "condition"]).size().unstack(fill_value=0))

# refresh automatically
time.sleep(REFRESH_S)
if hasattr(st, "rerun"):
    st.rerun()
else:
    st.experimental_rerun()