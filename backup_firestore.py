import firebase_admin
from firebase_admin import credentials, firestore
import json
from datetime import datetime
import os

# Initialize Firebase
cred = credentials.Certificate("iotshss-firebase-adminsdk-fbsvc-feb96a7009.json")  # Make sure it's in the same folder
firebase_admin.initialize_app(cred)
db = firestore.client()

# Create output folder if it doesn't exist
backup_folder = "Backups"
os.makedirs(backup_folder, exist_ok=True)

# Get all documents from 'sensor_data'
docs = db.collection('sensor_data').stream()

# Convert to list of dicts
data = [doc.to_dict() for doc in docs]

# Filename with current date
date_str = datetime.now().strftime("%Y-%m-%d")
backup_file = os.path.join(backup_folder, f"sensor_backup_{date_str}.json")

# Save to JSON file
with open(backup_file, "w") as f:
    json.dump(data, f, indent=2)

print(f"Backup complete: {backup_file}")
