from flask import Flask, request, jsonify
from pymongo import MongoClient
from datetime import datetime
from datetime import timedelta
import json
import os
import sys

# initialize environment variables
mongo_uri = os.getenv('MONGO_URI', None)
mongo_db = os.getenv('MONGO_DB', None)
mongo_col_device = os.getenv('MONGO_COL_DEV', None)
mongo_col_user = os.getenv('MONGO_COL_USER', None)
if mongo_uri is None or mongo_db is None or mongo_col_device is None or mongo_col_user is None:
    print('MONGO_URI is required')
    sys.exit(1)

# initialize app
app = Flask(__name__)
mongo_client = MongoClient(mongo_uri)
print("MongoDB connected: ", mongo_uri)

# REST API: station
@app.route('/api/station/<station_id>', methods=['GET'])
def query_station(station_id):
    resp = {}
    # extract parameters
    if station_id is None:
        resp['status'] = 'error'
        resp['message'] = 'station_id is required'
        return jsonify(resp)
    rssi_cond = int(request.args.get('rssi', -60))
    mins_cond = int(request.args.get('mins', 10))
    # database connection
    db = mongo_client[mongo_db]
    db_dev_col = db[mongo_col_device]
    print(db_dev_col.count_documents({}))
    # choose only valid data, sort by timestamp
    docs = db_dev_col.find({"station": station_id,
                            "timestamp": {"$gt": datetime.now() - timedelta(minutes=mins_cond)},
                            "rssi": {"$gt": rssi_cond}}).sort("timestamp", -1)
    resp['status'] = 'ok'
    resp['station'] = station_id
    resp['data'] = []
    devices = []
    for doc in docs:
        if doc['device'] in devices:
            continue
        devices.append(doc['device'])
        doc_data = {}
        doc_data['timestamp'] = doc['timestamp'].isoformat()
        doc_data['device'] = doc['device']
        doc_data['rssi'] = doc['rssi']
        resp['data'].append(doc_data)
    return jsonify(resp)

# REST API: asset
@app.route('/api/asset/<asset_id>', methods=['GET'])
def query_asset(asset_id):
    resp = {}
    if asset_id is None:
        resp['status'] = 'error'
        resp['message'] = 'asset_id is required'
        return jsonify(resp)
    rssi_cond = int(request.args.get('rssi', -60))
    mins_cond = int(request.args.get('mins', 60))    
    # database connection
    db = mongo_client[mongo_db]
    db_dev_col = db[mongo_col_device]
    # choose only valid data, sort by timestamp
    docs = db_dev_col.find({"device": asset_id,
                            "timestamp": {"$gt": datetime.now() - timedelta(minutes=mins_cond)},
                            "rssi": {"$gt": rssi_cond}}).sort("timestamp", -1)
    resp['status'] = 'ok'
    resp['asset'] = asset_id
    resp['data'] = []
    for doc in docs:
        doc_data = {}
        doc_data['timestamp'] = doc['timestamp'].isoformat()
        doc_data['station'] = doc['station']
        doc_data['rssi'] = doc['rssi']
        resp['data'].append(doc_data)
    return jsonify(resp)

if __name__ == "__main__":
    app.run(debug=True)