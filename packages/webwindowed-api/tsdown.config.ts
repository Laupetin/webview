import { defineConfig } from "tsdown";

export default defineConfig({
  platform: "browser",
  exports: true,
  tsconfig: "./tsconfig.web.json",
  dts: true,
});
