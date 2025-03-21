import streamlit as st
import pandas as pd
import numpy as np
import requests
import json
from langchain_google_genai import ChatGoogleGenerativeAI
from langchain.schema import HumanMessage, AIMessage
import os

# Set up the LLM model
llm = ChatGoogleGenerativeAI(model="gemini-2.0-flash", api_key='')

# chat history
if "messages" not in st.session_state:
    st.session_state.messages = []

# Function to fetch user data from an API
def fetch_asset_data(asset_id):
    url = f"http://localhost:5000/api/asset/{asset_id}?mins=100000&rssi=-100"
    response = requests.get(url)
    return response.json()

# UI for API key
st.title('GenAI App')
st.sidebar.title('Settings')
asset_id = st.sidebar.selectbox('Asset ID',['Asset-0', 'Asset-1', 'Asset-2', 'Asset-3'])
asset_data = fetch_asset_data(asset_id)
context_json = json.dumps(asset_data, indent=2)

# UI for chatbot
chat_ui, data_ui = st.tabs(["Chatbot", "Raw data"])

with chat_ui:
    for msg in st.session_state.messages:
        role = "user" if isinstance(msg, HumanMessage) else "assistant"
        with st.chat_message(role):
            st.write(msg.content)

    user_input = st.chat_input("Question: ...")
    if user_input:
        st.session_state.messages.append(HumanMessage(content=user_input))

        # Construct prompt with context
        prompt = f'''
        Now, answer the following question based on the JSON data:
        {user_input}
        Do not give JSON data in the answer.

        Here is the info about the asset tracking with Blutooth Low Energy (BLE) technology.
        {context_json}

        JSON data contains the following fields:
        "asset" is the asset ID
        "status" is the status of query operation
        "data" is the record of observations containing the following three fields:
            "timestamp" is the time of observation
            "rssi" is the received signal strength indicator, bigger is closer to the station
            "station" is the id of BLE station that made the observation

        '''

        # Generate response
        response = llm.invoke([HumanMessage(content=prompt)])
        st.session_state.messages.append(AIMessage(content=response.content))
        with st.chat_message("assistant"):
            st.write(response.content)

with data_ui:
    st.json(asset_data)