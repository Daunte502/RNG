<template>
  <v-container class="container">
    <v-row class="row" justify="center">
      <v-col class="col col1" align="center">
        <v-card
          title="LED A"
          width="150"
          density="compact"
          border
          flat
          rounded="md"
        >
          <v-divider></v-divider>
          <v-card-item>
            <v-icon
              v-if="payload.ledA == 0"
              size="50"
              icon="mdi:mdi-lightbulb"
            ></v-icon>
            <v-icon
              v-else
              size="50"
              icon="mdi:mdi-lightbulb-on"
              color="yellow"
            ></v-icon>
          </v-card-item>
          <v-card-item>
            <v-btn
              class="text-caption"
              text="Toggle"
              variant="tonal"
              color="primary"
              density="compact"
              @click="toggle('LED A')"
            ></v-btn>
          </v-card-item>
        </v-card>
      </v-col>

      <v-col class="col col2" align="center">
        <v-card
          title="LED B"
          width="150"
          density="compact"
          border
          flat
          rounded="md"
        >
          <v-divider></v-divider>
          <v-card-item>
            <v-icon
              v-if="payload.ledB == 0"
              size="50"
              icon="mdi:mdi-lightbulb"
            ></v-icon>
            <v-icon
              v-else
              size="50"
              icon="mdi:mdi-lightbulb-on"
              color="yellow"
            ></v-icon>
          </v-card-item>
          <v-card-item>
            <v-btn
              class="text-caption"
              text="Toggle"
              variant="tonal"
              color="primary"
              density="compact"
              @click="toggle('LED B')"
            ></v-btn>
          </v-card-item>
        </v-card>
      </v-col>
    </v-row>

    <v-row class="row" justify="center">
      <v-col class="bg-black col col3" align="center">
        <span class="digit text-error">
          <span v-if="payload.number == 0">&#x1fbf0;</span>
          <span v-else-if="payload.number == 1">&#x1fbf1;</span>
          <span v-else-if="payload.number == 2">&#x1fbf2;</span>
          <span v-else-if="payload.number == 3">&#x1fbf3;</span>
          <span v-else-if="payload.number == 4">&#x1fbf4;</span>
          <span v-else-if="payload.number == 5">&#x1fbf5;</span>
          <span v-else-if="payload.number == 6">&#x1fbf6;</span>
          <span v-else-if="payload.number == 7">&#x1fbf7;</span>
          <span v-else-if="payload.number == 8">&#x1fbf8;</span>
          <span v-else-if="payload.number == 9">&#x1fbf9;</span>
        </span>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup>
import { onMounted } from "vue";
import { storeToRefs } from "pinia";
import { useMqttStore } from "@/store/mqttStore";

const Mqtt = useMqttStore();
const { payload } = storeToRefs(Mqtt);

onMounted(() => {
  Mqtt.connect();
});

const toggle = (name) => {
  Mqtt.toggleLED(name); // publishes to 620167361_sub
};
</script>

<style scoped>
@import url("https://fonts.googleapis.com/css2?family=Noto+Sans+Symbols+2&display=swap");

.digit {
  font-family: "Noto Sans Symbols 2";
  font-size: 250px;
}

.container {
  height: 100%;
}

.row {
  width: 100%;
  margin: 10px 0px;
  padding: 10px;
}

.col {
  margin: 0px 10px;
}

.col3 {
  max-width: 270px;
  height: 320px;
  border: 2px solid lightslategray;
}

.col1,
.col2 {
  max-width: 200px;
}
</style>