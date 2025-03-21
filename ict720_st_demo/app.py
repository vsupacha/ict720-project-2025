import streamlit as st
import pandas as pd
import requests
import json
import time

# initialize session state
if st.session_state.get('first_run', None) is None:
    time.sleep(5) # very long exe at first
    st.session_state['first_run'] = False

st.write('# Hello, world')

# query data from asset API
devices = ['Asset-0', 'Asset-1', 'Asset-2', 'Asset-3', 'Asset-A', 'Asset-B', 'Asset-C']
base_url = 'http://localhost:5000/api/asset/'
raw_data = {}
for device in devices:
    url = base_url + device + '?mins=100000&rssi=-100'
    resp = requests.get(url)
    data = json.loads(resp.text)
    raw_data[device] = data['data']
print(raw_data)
# Remove devices with no data
not_found_devices = []
for device in raw_data.keys():
    if len(raw_data[device]) == 0:
        not_found_devices.append(device)
for not_found_device in not_found_devices:
    print('Removing ' + not_found_device)
    del raw_data[not_found_device]
print(raw_data)

# re-organize the data into table format
rssi_data = []
idx = 0
for device in raw_data.keys():
    for data in raw_data[device]:
        record = []
        record.append(data['timestamp'])
        record.append(device)
        record.append(data['station'])
        record.append(data['rssi'])
        record.append(0)
        rssi_data.append(record)
print(len(rssi_data), rssi_data)

# import the data into a pandas dataframe
import pandas as pd
from datetime import datetime
df = pd.DataFrame(rssi_data, columns=['timestamp', 'device', 'station', 'rssi', 'label'])
df['timestamp'] = pd.to_datetime(df['timestamp'], format='mixed')

# group data by timestamp
station_list = df['station'].unique()
device_list = df['device'].unique()
df_dicts = {}
for station in station_list:
    for device in device_list:
        sub_df = df[(df['station'] == station) & (df['device'] == device)]
        if sub_df.empty:
            continue
        rssi_values = sub_df[['timestamp','rssi']].resample('1min', on='timestamp').mean()
        rssi_values = rssi_values.dropna().reset_index()
        idx = station + '+' + device
        df_dicts[idx] = rssi_values

for key in df_dicts.keys():
    st.subheader(key)
    st.write(df_dicts[key])