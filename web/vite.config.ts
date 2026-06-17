import { defineConfig } from 'vitest/config';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      '/auth': { target: 'http://34.143.65.129:8080', changeOrigin: true },
      '/teams': { target: 'http://34.143.65.129:8080', changeOrigin: true },
      '/logs': { target: 'http://34.143.65.129:8080', changeOrigin: true },
    },
  },
  test: {
    environment: 'jsdom',
    globals: true,
  },
});
