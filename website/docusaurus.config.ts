import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

const config: Config = {
  title: 'BASICGOL',
  tagline: 'A legibilidade do BASIC e do Portugol, com a portabilidade de C11.',
  url: process.env.DOCS_URL ?? 'https://havaianasdestruido.github.io',
  baseUrl: process.env.DOCS_BASE_URL ?? '/BASICGOL/',
  organizationName: 'havaianasdestruido',
  projectName: 'BASICGOL',
  trailingSlash: true,
  onBrokenLinks: 'throw',
  favicon: 'img/logo.svg',
  i18n: {defaultLocale: 'pt-BR', locales: ['pt-BR']},
  markdown: {mermaid: true, hooks: {onBrokenMarkdownLinks: 'throw'}},
  themes: [
    '@docusaurus/theme-mermaid',
    [
      '@easyops-cn/docusaurus-search-local',
      {
        hashed: true,
        language: ['pt', 'en'],
        indexBlog: false,
        docsRouteBasePath: '/',
        searchBarShortcutHint: true,
        highlightSearchTermsOnTargetPage: true,
      },
    ],
  ],
  presets: [
    [
      'classic',
      {
        docs: {
          routeBasePath: '/',
          numberPrefixParser: false,
          sidebarPath: './sidebars.ts',
          showLastUpdateTime: false,
          breadcrumbs: true,
        },
        blog: false,
        pages: false,
        theme: {customCss: './src/css/custom.css'},
      } satisfies Preset.Options,
    ],
  ],
  themeConfig: {
    colorMode: {defaultMode: 'light', respectPrefersColorScheme: true},
    image: 'img/logo.svg',
    navbar: {
      title: 'BASICGOL',
      logo: {alt: 'BASICGOL', src: 'img/logo.svg'},
      items: [
        {to: '/guias/comecar', label: 'Começar', position: 'left'},
        {to: '/referencia/', label: 'Referência', position: 'left'},
        {to: '/testes/', label: 'Testes', position: 'left'},
        {to: '/internos/', label: 'Internos', position: 'left'},
        {to: '/projeto/', label: 'Especificação', position: 'left'},
        {href: 'https://github.com/havaianasdestruido/BASICGOL', label: 'GitHub', position: 'right'},
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {title: 'Documentação', items: [
          {label: 'Primeiros passos', to: '/guias/comecar'},
          {label: 'Referência viva', to: '/referencia/'},
          {label: 'Testes', to: '/testes/'},
        ]},
        {title: 'Projeto', items: [
          {label: 'Especificação', to: '/projeto/'},
          {label: 'Compatibilidade', to: '/projeto/11-matriz-compatibilidade'},
          {label: 'GitHub', href: 'https://github.com/havaianasdestruido/BASICGOL'},
        ]},
        {title: 'Manutenção', items: [
          {label: 'Como o site é gerado', to: '/guias/publicacao'},
          {label: 'Inventário JSON', href: 'pathname:///api/inventario.json'},
        ]},
      ],
      copyright: `BASICGOL · Apache-2.0 · Documentação gerada do código e dos testes.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
      additionalLanguages: ['c', 'cpp', 'bash', 'python', 'json', 'yaml'],
    },
    mermaid: {theme: {light: 'neutral', dark: 'dark'}},
    tableOfContents: {minHeadingLevel: 2, maxHeadingLevel: 3},
  } satisfies Preset.ThemeConfig,
};

export default config;
