import { useQuery } from '@tanstack/react-query';
import axios from 'axios';
import { getMyTeam } from '../api/teams';
import { mapTeam } from '../api/mappers';

function is404(error: unknown): boolean {
  return axios.isAxiosError(error) && error.response?.status === 404;
}

export function useTeam() {
  const query = useQuery({
    queryKey: ['team'],
    queryFn: getMyTeam,
    select: mapTeam,
    staleTime: 5 * 60_000,
    retry: (failureCount, error) => !is404(error) && failureCount < 1,
  });

  // 서버는 팀 미소속이면 GET /teams/me 에 404를 반환한다
  const noTeam = query.isError && is404(query.error);

  return {
    team: query.data?.team,
    members: query.data?.members,
    isPending: query.isPending,
    isError: query.isError && !noTeam,
    noTeam,
    refetch: query.refetch,
  };
}
