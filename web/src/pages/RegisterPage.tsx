import { useState, type FormEvent } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { useMutation } from '@tanstack/react-query';
import axios from 'axios';
import * as authApi from '../api/auth';

export function RegisterPage() {
  const navigate = useNavigate();
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [passwordConfirm, setPasswordConfirm] = useState('');
  const [error, setError] = useState<string | null>(null);

  const mutation = useMutation({
    mutationFn: () => authApi.register(username, password),
    onSuccess: () => {
      // 서버 register 응답에 토큰이 없으므로 자동 로그인 없이 /login으로 이동
      navigate('/login', { replace: true, state: { registered: true, username } });
    },
    onError: (err: unknown) => {
      if (
        axios.isAxiosError(err) &&
        (err.response?.status === 409 || err.response?.status === 400)
      ) {
        setError('이미 사용 중인 사용자명입니다.');
      } else {
        setError('회원가입에 실패했습니다. 잠시 후 다시 시도해 주세요.');
      }
    },
  });

  function handleSubmit(e: FormEvent) {
    e.preventDefault();
    if (!username || !password) {
      setError('사용자명과 비밀번호를 입력해 주세요.');
      return;
    }
    if (password !== passwordConfirm) {
      setError('비밀번호가 일치하지 않습니다.');
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
              autoComplete="new-password"
              className="rounded border border-border bg-raised px-3 py-2 font-mono text-text-primary"
            />
          </label>
          <label className="flex flex-col gap-1">
            <span className="text-xs text-text-secondary">비밀번호 확인</span>
            <input
              type="password"
              value={passwordConfirm}
              onChange={(e) => setPasswordConfirm(e.target.value)}
              autoComplete="new-password"
              className="rounded border border-border bg-raised px-3 py-2 font-mono text-text-primary"
            />
          </label>
          {error && <p className="text-xs text-danger">{error}</p>}
          <button
            type="submit"
            disabled={mutation.isPending}
            className="mt-2 rounded bg-accent-soft px-3 py-2 font-semibold text-text-primary hover:bg-accent hover:text-bg disabled:cursor-not-allowed disabled:opacity-60 disabled:hover:bg-accent-soft disabled:hover:text-text-primary"
          >
            {mutation.isPending ? '처리 중...' : '회원가입'}
          </button>
        </form>
        <p className="mt-4 text-center text-xs text-text-muted">
          이미 계정이 있나요?{' '}
          <Link to="/login" className="text-accent hover:underline">
            로그인
          </Link>
        </p>
      </div>
    </div>
  );
}
