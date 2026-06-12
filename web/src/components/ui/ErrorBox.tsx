interface ErrorBoxProps {
  message: string;
  onRetry: () => void;
}

export function ErrorBox({ message, onRetry }: ErrorBoxProps) {
  return (
    <div className="rounded border border-border bg-surface p-6 text-center">
      <p className="mb-3 text-danger">{message}</p>
      <button
        type="button"
        onClick={onRetry}
        className="rounded border border-border px-3 py-1.5 text-text-secondary hover:bg-raised hover:text-text-primary"
      >
        다시 시도
      </button>
    </div>
  );
}
