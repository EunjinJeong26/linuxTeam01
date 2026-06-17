import { client, USE_MOCK } from './client';
import { delay } from './mock/data';
import type { ApiLoginResponse, ApiRegisterResponse } from '../types';

export async function login(
  username: string,
  password: string,
): Promise<ApiLoginResponse> {
  if (USE_MOCK) {
    await delay();
    // mock에서는 항상 jin(user_id=1)으로 로그인
    return { token: 'mock-token', user_id: 1, username };
  }
  const { data } = await client.post<ApiLoginResponse>('/auth/login', {
    username,
    password,
  });
  return data;
}

export async function register(
  username: string,
  password: string,
): Promise<ApiRegisterResponse> {
  if (USE_MOCK) {
    await delay();
    return { user_id: 1, username };
  }
  const { data } = await client.post<ApiRegisterResponse>('/auth/register', {
    username,
    password,
  });
  return data;
}
