import { NavLink, Link } from 'react-router-dom';
import { useAuth } from '../../hooks/useAuth';
import { useTeam } from '../../hooks/useTeam';

const navItems = [
  { to: '/', label: '타임라인' },
  { to: '/me', label: '내 로그' },
  { to: '/team', label: '팀' },
];

export function Header() {
  const { user, logout } = useAuth();
  const { team } = useTeam();

  return (
    <header className="fixed top-0 left-0 right-0 z-10 h-12 border-b border-border bg-surface">
      <div className="mx-auto flex h-full max-w-[960px] items-center justify-between px-4">
        <Link to="/" className="font-mono font-semibold text-accent">
          devlog
          {team && (
            <span className="font-normal text-text-secondary"> / {team.teamName}</span>
          )}
        </Link>
        <div className="flex items-center gap-4">
          <nav className="flex items-center gap-3">
            {navItems.map((item) => (
              <NavLink
                key={item.to}
                to={item.to}
                end={item.to === '/'}
                className={({ isActive }) =>
                  isActive
                    ? 'text-accent'
                    : 'text-text-secondary hover:text-text-primary'
                }
              >
                {item.label}
              </NavLink>
            ))}
          </nav>
          <span className="font-mono text-xs text-text-secondary">
            {user?.username}
          </span>
          <button
            type="button"
            onClick={logout}
            className="rounded border border-border px-2 py-1 text-xs text-text-secondary hover:bg-raised hover:text-text-primary"
          >
            로그아웃
          </button>
        </div>
      </div>
    </header>
  );
}
