// Your Firebase config object
const firebaseConfig = {
  apiKey: "AIzaSyDOIz_rbSwWV1cA0d4Gtu0MsVClYVMKaNE",
  authDomain: "iotshss.firebaseapp.com",
  projectId: "iotshss",
  storageBucket: "iotshss.firebasestorage.app",
  messagingSenderId: "128983813018",
  appId: "1:128983813018:web:6e99d6504234e9faba4823",
  measurementId: "G-NB9GP83DT6",
};
// Initialize Firebase
firebase.initializeApp(firebaseConfig);
const db = firebase.firestore();

console.log("Firebase initialized.");

async function loadSensorData() {
  db.collection("sensor_data")
    .orderBy("timestamp", "desc")
    .limit(10)
    .onSnapshot(
      (snapshot) => {
        console.log(`Realtime update: ${snapshot.size} documents.`);

        if (snapshot.empty) {
          console.warn("No documents found in 'sensor_data' collection.");
          return;
        }

        const timestamps = [];
        const gasLevels = [];
        const lightLevels = [];
        const motions = [];
        const soilMoistures = [];
        const warnings = [];

        // Always reverse so it's from oldest to newest
        const docs = snapshot.docs.reverse();

        docs.forEach((doc) => {
          const data = doc.data();
          console.log(`Document ID: ${doc.id}, Data:`, data);

          try {
            const ts = new Date(parseInt(doc.id)).toLocaleTimeString();
            timestamps.push(data.timestamp ?? 0);
            gasLevels.push(data.gas_level ?? 0);
            lightLevels.push(data.light_level ?? 0);
            motions.push(data.pir_motion ?? 0);
            soilMoistures.push(data.soil_moisture ?? 0);
            warnings.push(data.warnings ?? 0);
          } catch (e) {
            console.error(`Error processing document ${doc.id}:`, e);
          }
        });

        // Recreate all charts
        createChart("gasChart", "Gas Level", timestamps, gasLevels);
        createChart("lightChart", "Light Level", timestamps, lightLevels);
        createChart("motionChart", "PIR Motion", timestamps, motions);
        createChart("soilChart", "Soil Moisture", timestamps, soilMoistures);
        createChart("warningsChart", "Warnings", timestamps, warnings);
      },
      (error) => {
        console.error("Error listening to real-time updates:", error);
      }
    );
}


let chartInstances = {};

function createChart(canvasId, label, labels, data) {
  if (chartInstances[canvasId]) {
    chartInstances[canvasId].destroy();
  }

  const ctx = document.getElementById(canvasId);
  chartInstances[canvasId] = new Chart(ctx, {
    type: "line",
    data: {
      labels: labels,
      datasets: [
        {
          label: label,
          data: data,
          fill: false,
          borderColor: "rgb(75, 192, 192)",
          tension: 0.1,
        },
      ],
    },
    options: {
      responsive: true,
      scales: {
        x: {
          title: { display: true, text: "Time" },
        },
        y: {
          beginAtZero: true,
          title: { display: true, text: label },
        },
      },
    },
  });
}

loadSensorData();
