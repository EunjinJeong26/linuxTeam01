import { useRef, useState } from 'react';
import { useTeam } from '../hooks/useTeam';
import { ErrorBox } from '../components/ui/ErrorBox';
import { Skeleton } from '../components/ui/Skeleton';

export function TeamPage() {
  const { team, members, isPending, isError, noTeam, refetch } = useTeam();
  const [copied, setCopied] = useState(false);
  const copyTimer = useRef<number | undefined>(undefined);

  function copyInviteCode() {
    if (!team) return;
    navigator.clipboard
      .writeText(team.inviteCode)
      .then(() => {
        setCopied(true);
        window.clearTimeout(copyTimer.current);
        copyTimer.current = window.setTimeout(() => setCopied(false), 2000);
      })
      .catch(() => {});
  }

  if (noTeam) {
    return (
      <section className="flex flex-col gap-4">
        <h2 className="text-lg font-semibold">팀 정보</h2>
        <p className="rounded border border-border bg-surface p-6 text-center text-text-muted">
          소속된 팀이 없습니다. CLI(<code className="font-mono">devlog team create/join</code>)로
          팀에 참가한 후 이용할 수 있습니다.
        </p>
      </section>
    );
  }

  if (isError) {
    return (
      <section className="flex flex-col gap-4">
        <h2 className="text-lg font-semibold">팀 정보</h2>
        <ErrorBox
          message="팀 정보를 불러오지 못했습니다."
          onRetry={() => void refetch()}
        />
      </section>
    );
  }

  if (isPending || !team || !members) {
    return (
      <section className="flex flex-col gap-4">
        <h2 className="text-lg font-semibold">팀 정보</h2>
        <Skeleton className="h-32 w-full" />
        <Skeleton className="h-40 w-full" />
      </section>
    );
  }

  const sortedMembers = [...members].sort((a, b) => {
    if (a.role !== b.role) return a.role === 'leader' ? -1 : 1;
    return a.username.localeCompare(b.username);
  });

  return (
    <section className="flex flex-col gap-4">
      <h2 className="text-lg font-semibold">팀 정보</h2>

      <div className="rounded border border-border bg-surface p-4">
        <div className="mb-4">
          <p className="mb-1 text-xs text-text-secondary">팀명</p>
          <p className="font-mono font-semibold">{team.teamName}</p>
        </div>
        <div>
          <p className="mb-1 text-xs text-text-secondary">초대 코드</p>
          <div className="flex items-center gap-3">
            <span className="font-mono text-xl font-semibold tracking-widest">
              {team.inviteCode}
            </span>
            <button
              type="button"
              onClick={copyInviteCode}
              className="rounded border border-border px-2 py-1 text-xs text-text-secondary hover:bg-raised hover:text-text-primary"
            >
              {copied ? '복사됨' : '복사'}
            </button>
          </div>
        </div>
      </div>

      <div className="rounded border border-border bg-surface p-4">
        <p className="mb-3 text-xs text-text-secondary">
          멤버 ({members.length}명)
        </p>
        <table className="w-full text-left">
          <thead>
            <tr className="text-xs text-text-muted">
              <th className="pb-2 font-normal">사용자명</th>
              <th className="pb-2 font-normal">역할</th>
            </tr>
          </thead>
          <tbody>
            {sortedMembers.map((member) => (
              <tr key={member.userId} className="border-t border-border">
                <td className="py-2 font-mono">{member.username}</td>
                <td className="py-2">
                  <span
                    className={`inline-flex rounded border px-1.5 py-0.5 font-mono text-xs ${
                      member.role === 'leader'
                        ? 'border-accent-soft text-accent'
                        : 'border-border text-text-secondary'
                    }`}
                  >
                    {member.role}
                  </span>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      <p className="text-center text-xs text-text-muted">
        팀 생성·참가·탈퇴는 CLI(
        <code className="font-mono">devlog team ...</code>)에서 가능합니다.
      </p>
    </section>
  );
}
