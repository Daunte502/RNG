import { defineStore } from "pinia";

export const useAppStore = defineStore("app", () => {
  const getFrequencies = async () => {
    const controller = new AbortController();
    const signal = controller.signal;
    setTimeout(() => controller.abort(), 60000);

    const URL = "http://127.0.0.1:5000/api/numberfrequency";

    try {
      const response = await fetch(URL, { method: "GET", signal });
      if (response.ok) {
        const data = await response.json();
        if (data?.status === "found") return data.data;
      } else {
        console.log(await response.text());
      }
    } catch (err) {
      console.error("getFrequencies error:", err.message);
    }
    return [];
  };

  const getOnCount = async (LED_Name) => {
    const controller = new AbortController();
    const signal = controller.signal;
    setTimeout(() => controller.abort(), 60000);

    const URL = "http://127.0.0.1:5000/api/oncount";
    const form = new FormData();
    form.append("LED_Name", LED_Name);

    try {
      const response = await fetch(URL, { method: "POST", body: form, signal });
      if (response.ok) {
        const data = await response.json();
        if (data?.status === "found") return data.data;
      } else {
        console.log(await response.text());
      }
    } catch (err) {
      console.error("getOnCount error:", err.message);
    }
    return 0;
  };

  return { getFrequencies, getOnCount };
});