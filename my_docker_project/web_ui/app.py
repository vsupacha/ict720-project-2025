import streamlit as st
import pandas as pd
import numpy as np
import requests
import os

# Get environment variables
rest_station_api = os.getenv('REST_STATION_URI', None)
rest_asset_api = os.getenv('REST_ASSET_URI', None)
if rest_station_api is None or rest_asset_api is None:
    raise ValueError('REST API variables are not set')

# page settings
st.title('My demo app')
st.sidebar.title('Settings')
station_id = st.sidebar.selectbox('Station', ['esp32', 'ESP32'])

# main page
url = rest_station_api + station_id + '?mins=100000&rssi=-100'
response = requests.get(url)
station_data = response.json()
df = pd.DataFrame(station_data)
st.write(df)