import axios from 'axios';
import { clearAuthStorage, TOKEN_KEY } from '../contexts/AuthContext';

export const USE_MOCK = import.meta.env.VITE_USE_MOCK === 'true';

export const client = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL || '',
});

client.interceptors.request.use((config) => {
  const token = localStorage.getItem(TOKEN_KEY);
  if (token) config.headers.Authorization = `Bearer ${token}`;
  return config;
});

client.interceptors.response.use(
  (res) => res,
  (err: unknown) => {
    if (axios.isAxiosError(err) && err.response?.status === 401) {
      clearAuthStorage();
      if (window.location.pathname !== '/login') {
        window.location.href = '/login';
      }
    }
    return Promise.reject(err);
  },
);
