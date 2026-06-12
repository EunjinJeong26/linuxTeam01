import { createContext, useCallback, useMemo, useState, type ReactNode } from 'react';
import type { User } from '../types';

export const TOKEN_KEY = 'devlog_token';
export const USER_ID_KEY = 'devlog_user_id';
const USERNAME_KEY = 'devlog_username';

interface AuthState {
  user: User | null;
  isAuthenticated: boolean;
  login: (token: string, userId: number, username: string) => void;
  logout: () => void;
}

export const AuthContext = createContext<AuthState | null>(null);

function readStoredUser(): User | null {
  const token = localStorage.getItem(TOKEN_KEY);
  const userId = localStorage.getItem(USER_ID_KEY);
  const username = localStorage.getItem(USERNAME_KEY);
  if (!token || !userId || !username) return null;
  return { userId: Number(userId), username };
}

export function clearAuthStorage(): void {
  localStorage.removeItem(TOKEN_KEY);
  localStorage.removeItem(USER_ID_KEY);
  localStorage.removeItem(USERNAME_KEY);
}

export function AuthProvider({ children }: { children: ReactNode }) {
  const [user, setUser] = useState<User | null>(readStoredUser);

  const login = useCallback((token: string, userId: number, username: string) => {
    localStorage.setItem(TOKEN_KEY, token);
    localStorage.setItem(USER_ID_KEY, String(userId));
    localStorage.setItem(USERNAME_KEY, username);
    setUser({ userId, username });
  }, []);

  const logout = useCallback(() => {
    clearAuthStorage();
    setUser(null);
  }, []);

  const value = useMemo<AuthState>(
    () => ({ user, isAuthenticated: user !== null, login, logout }),
    [user, login, logout],
  );

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}
