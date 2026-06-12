import { client, USE_MOCK } from './client';
import { delay, mockTeam } from './mock/data';
import type { ApiMyTeamResponse } from '../types';

export async function getMyTeam(): Promise<ApiMyTeamResponse> {
  if (USE_MOCK) {
    await delay();
    return mockTeam;
  }
  const { data } = await client.get<ApiMyTeamResponse>('/teams/me');
  return data;
}
