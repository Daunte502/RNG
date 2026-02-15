<template>
  <v-container fluid align="center">
    <v-row justify="center" style="max-width: 1200px">
      <v-col cols="12" align="center">
        <canvas id="myChart"></canvas>
      </v-col>

      <v-col cols="12" align="center">
        <v-btn
          text="Refresh Graph"
          flat
          color="secondary"
          variant="outlined"
          @click="updateGraph()"
        ></v-btn>
      </v-col>
    </v-row>

    <v-row style="max-width: 1200px">
      <v-divider class="mt-5 mb-5"></v-divider>
    </v-row>

    <v-row justify="center" style="max-width: 1200px">
      <v-card class="ma-2" subtitle="LED A" width="150" flat border align="center">
        <v-card-item class="pa-0">Turned on</v-card-item>
        <v-card-item class="pa-0">
          <span class="text-h5 text-primary font-weight-bold">{{ led_A }}</span>
        </v-card-item>
        <v-card-item class="pa-0">times</v-card-item>
        <v-card-item>
          <v-btn
            text="Update"
            class="ma-1 text-caption"
            rounded="pill"
            flat
            color="secondary"
            variant="tonal"
            @click="updateLEDCount('ledA')"
          ></v-btn>
        </v-card-item>
      </v-card>

      <v-card class="ma-2" subtitle="LED B" width="150" flat border align="center">
        <v-card-item class="pa-0">Turned on</v-card-item>
        <v-card-item class="pa-0">
          <span class="text-h5 text-primary font-weight-bold">{{ led_B }}</span>
        </v-card-item>
        <v-card-item class="pa-0">times</v-card-item>
        <v-card-item>
          <v-btn
            text="Update"
            class="ma-1 text-caption"
            rounded="pill"
            flat
            color="secondary"
            variant="tonal"
            @click="updateLEDCount('ledB')"
          ></v-btn>
        </v-card-item>
      </v-card>
    </v-row>
  </v-container>
</template>

<script setup>
import { onMounted, ref } from "vue";
import Chart from "chart.js/auto";
import { useAppStore } from "@/store/appStore";

const AppStore = useAppStore();

const led_A = ref(0);
const led_B = ref(0);

let chart = null;

const data = {
  labels: ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"],
  datasets: [
    {
      label: "Frequency",
      data: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
      borderColor: "#1ECBE1",
      backgroundColor: "#4BD5E7",
      borderWidth: 2,
      borderRadius: 5,
      borderSkipped: false,
    },
  ],
};

const config = {
  type: "bar",
  data,
  options: {
    responsive: true,
    plugins: {
      legend: { position: "top" },
      title: { display: true, text: "Random Number Frequency" },
    },
  },
};

const updateData = (chartObj, labels, newData) => {
  chartObj.data.labels = labels;
  chartObj.data.datasets[0].data = newData;
  chartObj.update();
};

onMounted(() => {
  const ctx = document.querySelector("#myChart");
  chart = new Chart(ctx, config);
});

const updateGraph = async () => {
  const result = await AppStore.getFrequencies();
  const labels = [];
  const points = [];

  if (result.length > 0) {
    result.forEach((obj) => {
      labels.push(String(obj.number));
      points.push(obj.frequency);
    });
    updateData(chart, labels, points);
  }
};

const updateLEDCount = async (name) => {
  const result = await AppStore.getOnCount(name);
  if (name === "ledA") led_A.value = result;
  if (name === "ledB") led_B.value = result;
};
</script>