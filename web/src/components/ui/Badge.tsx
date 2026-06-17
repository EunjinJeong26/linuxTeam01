import type { ReactNode } from 'react';

const variantClass = {
  git: 'text-git',
  sys: 'text-sys',
} as const;

interface BadgeProps {
  variant: keyof typeof variantClass;
  children: ReactNode;
}

export function Badge({ variant, children }: BadgeProps) {
  return (
    <span
      className={`inline-flex items-center rounded border border-border bg-raised px-1.5 py-0.5 font-mono text-xs ${variantClass[variant]}`}
    >
      {children}
    </span>
  );
}
