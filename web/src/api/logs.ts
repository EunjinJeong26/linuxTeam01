import { client, USE_MOCK } from './client';
import { delay, mockLogsFor } from './mock/data';
import { USER_ID_KEY } from '../contexts/AuthContext';
import type { ApiLog } from '../types';

export async function getTeamLogs(teamId: number, date: string): Promise<ApiLog[]> {
  if (USE_MOCK) {
    await delay();
    return mockLogsFor(teamId, date);
  }
  const { data } = await client.get<ApiLog[]>('/logs', {
    params: { teamId, date, my: false },
  });
  return data;
}

export async function getMyLogs(teamId: number, date: string): Promise<ApiLog[]> {
  if (USE_MOCK) {
    await delay();
    const myUserId = Number(localStorage.getItem(USER_ID_KEY));
    return mockLogsFor(teamId, date, myUserId);
  }
  const { data } = await client.get<ApiLog[]>('/logs', {
    params: { teamId, date, my: true },
  });
  return data;
}
