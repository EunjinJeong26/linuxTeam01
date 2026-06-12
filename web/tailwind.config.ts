import type { Config } from 'tailwindcss';

export default {
  content: ['./index.html', './src/**/*.{ts,tsx}'],
  theme: {
    extend: {
      colors: {
        bg: '#0E1116',
        surface: '#161B22',
        raised: '#1D232C',
        border: '#2A313B',
        text: {
          primary: '#E6EDF3',
          secondary: '#9BA6B2',
          muted: '#5C6670',
        },
        accent: {
          DEFAULT: '#3FB68B',
          soft: '#1E5C46',
        },
        git: '#E9B259',
        sys: '#6CA9E8',
        danger: '#E2645A',
      },
      fontFamily: {
        mono: ['"JetBrains Mono"', 'ui-monospace', 'monospace'],
        sans: ['"Pretendard Variable"', 'Pretendard', 'sans-serif'],
      },
      borderRadius: {
        DEFAULT: '6px',
      },
    },
  },
  plugins: [],
} satisfies Config;
