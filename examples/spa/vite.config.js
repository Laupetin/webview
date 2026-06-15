import { fileURLToPath, URL } from "node:url";

import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import PluginCppHeader from "@laupetin/vite-plugin-cpp-header";

// https://vite.dev/config/
export default defineConfig({
  plugins: [vue(), PluginCppHeader()],
  resolve: {
    alias: {
      "@": fileURLToPath(new URL("./src", import.meta.url)),
    },
  },
});
