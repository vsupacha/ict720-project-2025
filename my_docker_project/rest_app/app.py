from flask import Flask
from pymongo import MongoClient
from datetime import datetime
import json

app = Flask(__name__)
mongo_client = MongoClient('my_docker_project-mongo-1', 27017,
                           username='root', password='example')

@app.route('/api/station/<station_id>', methods=['GET'])
def query_station(station_id):
    db = mongo_client.taist_db
    db_col = db.ble_logs
    resp = {}
    if station_id is None:
        resp['status'] = 'error'
        resp['message'] = 'station_id is required'
        return json.dumps(resp)
    data = db_col.find({"station": station_id})
    resp['status'] = 'ok'
    resp['station'] = station_id
    resp['data'] = []
    for record in data:
        record_data = {}
        record_data['timestamp'] = record['timestamp'].isoformat()
        record_data['device'] = record['device']
        record_data['rssi'] = record['rssi']
        resp['data'].append(record_data)
    return json.dumps(resp)

@app.route('/api/asset/<asset_id>', methods=['GET'])
def query_asset(asset_id):
    db = mongo_client.taist_db
    db_col = db.ble_logs
    resp = {}
    if asset_id is None:
        resp['status'] = 'error'
        resp['message'] = 'asset_id is required'
        return json.dumps(resp)
    data = db_col.find({"device": asset_id})
    resp['status'] = 'ok'
    resp['asset'] = asset_id
    resp['data'] = []
    for record in data:
        record_data = {}
        record_data['timestamp'] = record['timestamp'].isoformat()
        record_data['station'] = record['station']
        record_data['rssi'] = record['rssi']
        resp['data'].append(record_data)
    return json.dumps(resp)

if __name__ == "__main__":
    app.run(debug=True)