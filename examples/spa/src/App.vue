<script setup lang="ts">
import { ref } from "vue";
import { getBinds, getEvents } from "@webwindowed/web-api";

const counter = ref(0);
const tick = ref(0);
const pathValue = ref<string | undefined>(undefined);

type Binds = {
  path(): Promise<string>;
};
type Events = {
  tick: number;
};

const { path } = getBinds<Binds>();
const { addEventListener } = getEvents<Events>();

path().then((res) => {
  pathValue.value = res;
});

addEventListener("tick", (value) => {
  tick.value = value;
});

function onIncrement() {
  counter.value++;
}
function onDecrement() {
  counter.value--;
}
</script>

<template>
  <main>
    <h1>Sample SPA</h1>
    <p>
      This is a
      <a href="https://vuejs.org/" target="_blank" rel="noopener">VueJS</a>
      single page application, built with Vite.
    </p>
    <div>
      <h3>Demonstration of frontend interactivity:</h3>
      <button @click="onIncrement">+</button>
      <button @click="onDecrement">−</button>
      <span>Counter: {{ counter }}</span>
    </div>
    <div>
      <h3>Demonstration of binds:</h3>
      <div>
        <div>C++ has the following working directory:</div>
        <div v-if="pathValue">{{ pathValue }}</div>
        <div v-else><i>loading</i></div>
      </div>
    </div>
    <div>
      <h3>Demonstration of events:</h3>
      <div>
        <div>C++ emits ticks as events:</div>
        <div>{{ tick }}</div>
      </div>
    </div>
  </main>
</template>

<style scoped></style>
