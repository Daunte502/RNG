import { defineStore } from "pinia";
import { ref } from "vue";

export const useMqttStore = defineStore(
  "mqtt",
  () => {
    // ====== CONFIG (YOUR LAB SETTINGS) ======
    const STUDENT_ID = "620167361";

    // WebSocket settings (Mosquitto must have a websockets listener)
    const host = ref("localhost");
    const port = ref(9001);

    const TOPIC_UPDATES = `${STUDENT_ID}_pub`;
    const TOPIC_COMMANDS = `${STUDENT_ID}_sub`;

    // STATES
    const mqtt = ref(null);
    const payload = ref({
      id: STUDENT_ID,
      timestamp: 1702566538,
      number: 0,
      ledA: 0,
      ledB: 0,
    });
    const payloadTopic = ref("");
    const subTopics = ref({});

    // SUBSCRIBE UTIL FUNCTIONS
    const sub_onSuccess = (response) => {
      const topic = response.invocationContext.topic;
      console.log(`MQTT: Subscribed to - ${topic}`);
      subTopics.value[topic] = "subscribed";
    };

    const sub_onFailure = (response) => {
      const topic = response.invocationContext.topic;
      console.log(
        `MQTT: Failed to subscribe to - ${topic}\nError message : ${response.errorMessage}`,
      );
    };

    const subscribe = (topic) => {
      try {
        const subscribeOptions = {
          onSuccess: sub_onSuccess,
          onFailure: sub_onFailure,
          invocationContext: { topic },
        };
        mqtt.value.subscribe(topic, subscribeOptions);
      } catch (error) {
        console.log(`MQTT: Unable to Subscribe ${error}`);
      }
    };

    // MQTT callbacks
    const onSuccess = () => {
      // connect acknowledgement received from the server
      console.log("MQTT: Connected (onSuccess)");
      // Subscribe after we are connected
      subscribe(TOPIC_UPDATES);
    };

    const onConnected = (reconnect, URI) => {
      console.log(`Connected to: ${URI} , Reconnect: ${reconnect}`);

      // Ensure subscription exists (safe to call multiple times)
      subscribe(TOPIC_UPDATES);

      if (reconnect) {
        const topics = Object.keys(subTopics.value);
        topics.forEach((topic) => subscribe(topic));
      }
    };

    const onConnectionLost = (response) => {
      if (response.errorCode !== 0) {
        console.log(`MQTT: Connection lost - ${response.errorMessage}`);
      }
    };

    const onFailure = (response) => {
      const hostVal = response.invocationContext.host;
      console.log(
        `MQTT: Connection to ${hostVal} failed.\nError message : ${response.errorMessage}`,
      );
    };

    const onMessageArrived = (response) => {
      try {
        payload.value = JSON.parse(response.payloadString);
        payloadTopic.value = response.destinationName;
        console.log(
          `Topic : ${payloadTopic.value}\nPayload : ${response.payloadString}`,
        );
      } catch (error) {
        console.log(`onMessageArrived Error: ${error}`);
      }
    };

    const makeid = (length) => {
      let result = "";
      const characters =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
      for (let i = 0; i < length; i++) {
        result += characters.charAt(Math.floor(Math.random() * characters.length));
      }
      return "IOT_F_" + result;
    };

    // UNSUBSCRIBE UTIL FUNCTIONS
    const unSub_onSuccess = (response) => {
      const topic = response.invocationContext.topic;
      console.log(`MQTT: Unsubscribed from - ${topic}`);
      delete subTopics.value[topic];
    };

    const unSub_onFailure = (response) => {
      const topic = response.invocationContext.topic;
      console.log(
        `MQTT: Failed to unsubscribe from - ${topic}\nError message : ${response.errorMessage}`,
      );
    };

    const unsubcribe = (topic) => {
      const unsubscribeOptions = {
        onSuccess: unSub_onSuccess,
        onFailure: unSub_onFailure,
        invocationContext: { topic },
      };
      mqtt.value.unsubscribe(topic, unsubscribeOptions);
    };

    const unsubcribeAll = () => {
      const topics = Object.keys(subTopics.value);
      topics.forEach((topic) => {
        const unsubscribeOptions = {
          onSuccess: unSub_onSuccess,
          onFailure: unSub_onFailure,
          invocationContext: { topic },
        };
        mqtt.value.unsubscribe(topic, unsubscribeOptions);
      });
      disconnect();
    };

    // PUBLISH UTIL FUNCTION
    const publish = (topic, body) => {
      const message = new Paho.MQTT.Message(body);
      message.destinationName = topic;
      mqtt.value.send(message);
    };

    const disconnect = () => {
      mqtt.value.disconnect();
    };

    const connect = () => {
      const IDstring = makeid(12);

      console.log(
        `MQTT: Connecting (WebSocket) to Server : ${host.value} Port : ${port.value}`,
      );

      mqtt.value = new Paho.MQTT.Client(host.value, port.value, "/mqtt", IDstring);

      const options = {
        timeout: 5,
        onSuccess,
        onFailure,
        invocationContext: { host: host.value, port: port.value },
        useSSL: false,
        reconnect: true,
      };

      mqtt.value.onConnectionLost = onConnectionLost;
      mqtt.value.onMessageArrived = onMessageArrived;
      mqtt.value.onConnected = onConnected;

      mqtt.value.connect(options);
    };

    // Lab helper: publish a toggle command
    const toggleLED = (deviceName) => {
      const message = JSON.stringify({ type: "toggle", device: deviceName });
      publish(TOPIC_COMMANDS, message);
    };

    return {
      payload,
      payloadTopic,
      host,
      port,
      connect,
      disconnect,
      subscribe,
      unsubcribe,
      unsubcribeAll,
      publish,
      toggleLED,
    };
  },
  { persist: true },
);