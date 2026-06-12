import { useState, type FormEvent } from 'react';
import { Link, useLocation, useNavigate } from 'react-router-dom';
import { useMutation } from '@tanstack/react-query';
import axios from 'axios';
import { useAuth } from '../hooks/useAuth';
import * as authApi from '../api/auth';

interface RegisterRedirectState {
  registered?: boolean;
  username?: string;
}

export function LoginPage() {
  const { login } = useAuth();
  const navigate = useNavigate();
  const location = useLocation();
  const redirectState = (location.state ?? null) as RegisterRedirectState | null;

  const [username, setUsername] = useState(redirectState?.username ?? '');
  const [password, setPassword] = useState('');
  const [error, setError] = useState<string | null>(null);

  const mutation = useMutation({
    mutationFn: () => authApi.login(username, password),
    onSuccess: (data) => {
      login(data.token, data.user_id, data.username);
      navigate('/', { replace: true });
    },
    onError: (err: unknown) => {
      if (axios.isAxiosError(err) && err.response && err.response.status < 500) {
        setError('사용자명 또는 비밀번호가 올바르지 않습니다');
      } else {
        setError('로그인에 실패했습니다. 잠시 후 다시 시도해 주세요.');
      }
    },
  });

  function handleSubmit(e: FormEvent) {
    e.preventDefault();
    if (!username || !password) {
      setError('사용자명과 비밀번호를 입력해 주세요.');
      return;
    }
    setError(null);
    mutation.mutate();
  }

  return (
    <div className="flex min-h-screen items-center justify-center px-4">
      <div className="w-full max-w-[360px] rounded border border-border bg-surface p-6">
        <h1 className="mb-6 text-center font-mono text-lg font-semibold text-accent">
          devlog
        </h1>
        {redirectState?.registered && (
          <p className="mb-4 rounded border border-accent-soft bg-accent-soft/30 px-3 py-2 text-xs text-accent">
            회원가입이 완료되었습니다. 로그인해 주세요.
          </p>
        )}
        <form onSubmit={handleSubmit} className="flex flex-col gap-3">
          <label className="flex flex-col gap-1">
            <span className="text-xs text-text-secondary">사용자명</span>
            <input
              type="text"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              autoComplete="username"
              className="rounded border border-border bg-raised px-3 py-2 font-mono text-text-primary"
            />
          </label>
          <label className="flex flex-col gap-1">
            <span className="text-xs text-text-secondary">비밀번호</span>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              autoComplete="current-password"
              className="rounded border border-border bg-raised px-3 py-2 font-mono text-text-primary"
            />
          </label>
          {error && <p className="text-xs text-danger">{error}</p>}
          <button
            type="submit"
            disabled={mutation.isPending}
            className="mt-2 rounded bg-accent-soft px-3 py-2 font-semibold text-text-primary hover:bg-accent hover:text-bg disabled:cursor-not-allowed disabled:opacity-60 disabled:hover:bg-accent-soft disabled:hover:text-text-primary"
          >
            {mutation.isPending ? '처리 중...' : '로그인'}
          </button>
        </form>
        <p className="mt-4 text-center text-xs text-text-muted">
          계정이 없나요?{' '}
          <Link to="/register" className="text-accent hover:underline">
            회원가입
          </Link>
        </p>
      </div>
    </div>
  );
}
