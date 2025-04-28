// Your Firebase config
const firebaseConfig = {
  apiKey: "AIzaSyDOIz_rbSwWV1cA0d4Gtu0MsVClYVMKaNE",
  authDomain: "iotshss.firebaseapp.com",
  projectId: "iotshss",
  storageBucket: "iotshss.firebasestorage.app",
  messagingSenderId: "128983813018",
  appId: "1:128983813018:web:6e99d6504234e9faba4823",
  measurementId: "G-NB9GP83DT6",
};

firebase.initializeApp(firebaseConfig);
const db = firebase.firestore();

let gasChart, lightChart, motionChart, soilChart, warningsChart, waterChart;

const opts = {
  angle: 0,
  lineWidth: 0.3,
  radiusScale: 1,
  pointer: {
    length: 0.6,
    strokeWidth: 0.035,
    color: "#000000",
  },
  staticZones: [
    { strokeStyle: "#30B32D", min: 0, max: 20 },
    { strokeStyle: "#FFDD00", min: 20, max: 40 },
    { strokeStyle: "#FFA500", min: 40, max: 60 },
    { strokeStyle: "#F03E3E", min: 60, max: 100 },
  ],
  limitMax: true,
  limitMin: true,
  colorStart: "#6FADCF",
  colorStop: "#8FC0DA",
  strokeColor: "#E0E0E0",
  generateGradient: true,
};

const target = document.getElementById("gaugeCanvas");
const gauge = new Gauge(target).setOptions(opts);
gauge.maxValue = 100;
gauge.setMinValue(0);
gauge.set(0);

window.onload = function () {
  setupCharts();
  listenRealtimeUpdates();
};

function setupCharts() {
  gasChart = createLineChart("gasChart", "Gas Level");
  lightChart = createLineChart("lightChart", "Light Level");
  motionChart = createLineChart("motionChart", "PIR Motion");
  soilChart = createLineChart("soilChart", "Soil Moisture");
  warningsChart = createLineChart("warningsChart", "Warnings");
  waterChart = createLineChart("waterChart", "Water Level");
}

function createLineChart(canvasId, label) {
  return new Chart(document.getElementById(canvasId).getContext("2d"), {
    type: "line",
    data: {
      labels: [],
      datasets: [
        {
          label: label,
          data: [],
          fill: false,
          borderColor: "rgb(75, 192, 192)",
          tension: 0.3,
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

function listenRealtimeUpdates() {
  db.collection("sensor_data")
    .orderBy("timestamp", "desc")
    .limit(10)
    .onSnapshot((snapshot) => {
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
      const waterLevels = [];

      snapshot.forEach((doc) => {
        const data = doc.data();
        const ts = new Date(parseInt(doc.id)).toLocaleTimeString();
        timestamps.push(data.timestamp ?? 0);
        gasLevels.push(data.gas_level ?? 0);
        lightLevels.push(data.light_level ?? 0);
        motions.push(data.pir_motion ?? 0);
        soilMoistures.push(data.soil_moisture ?? 0);
        warnings.push(data.warnings ?? 0);
        waterLevels.push(data.water_level ?? 0);
      });

      updateChart(gasChart, timestamps, gasLevels);
      updateChart(lightChart, timestamps, lightLevels);
      updateChart(motionChart, timestamps, motions);
      updateChart(soilChart, timestamps, soilMoistures);
      updateChart(warningsChart, timestamps, warnings);
      updateChart(waterChart, timestamps, waterLevels);

      if (gasLevels.length > 0) {
        gauge.set(gasLevels[0]);
      }
    });
}

function updateChart(chart, labels, data) {
  chart.data.labels = labels.reverse();
  chart.data.datasets[0].data = data.reverse();
  chart.update();
}
