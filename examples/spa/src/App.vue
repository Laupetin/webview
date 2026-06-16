<script setup>
import { ref } from "vue";

const counter = ref(0);
const path = ref(undefined);

window.webwindowedBinds.path().then((res) => {
  path.value = res;
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
        <div v-if="path">{{ path }}</div>
        <div v-else><i>loading</i></div>
      </div>
    </div>
  </main>
</template>

<style scoped></style>
