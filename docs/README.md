# Philosophers Project Documentation

This directory contains interactive documentation for the Philosophers project.

## Overview

The documentation is structured as a set of HTML pages with interactive elements built using:
- Materialize CSS for styling
- PixiJS for interactive visualizations
- Prism.js for code highlighting

## Getting Started

To view the documentation, open the `index.html` file in a web browser:

```
cd docs
open index.html
```

Or use a local server (for better performance):

```
cd docs
python3 -m http.server
```

Then navigate to `http://localhost:8000` in your web browser.

## Documentation Structure

The documentation is organized into several sections:

1. **Home** (`index.html`): Introduction to the project
2. **Overview** (`overview.html`): Explanation of the Dining Philosophers problem
3. **Code Explained** (`code-explained.html`): Detailed explanation of the code implementation
4. **Visualization** (`visualization.html`): Interactive simulation of the philosophers
5. **Documentation** (`documentation.html`): API reference for all functions and data structures
6. **Game** (`game.html`): Interactive 3D game where users control philosophers
7. **Learning Paths** (`learning-paths.html`): Different approaches to learning about the problem

## Directory Structure

```
docs/
├── css/
│   └── style.css
├── img/
│   └── philosophers_bg.jpg
├── js/
│   ├── main.js
│   ├── philosophers-game.js
│   ├── generate-banner.js
│   └── code-highlight.js
├── index.html
├── overview.html
├── code-explained.html
├── visualization.html
├── documentation.html
├── game.html
├── learning-paths.html
└── banner-generator.html
```

## Features

- **Interactive Visualization**: See the philosophers in action with color-coded states
- **Code Highlighting**: All code snippets are syntax-highlighted for readability
- **Responsive Design**: Works on desktop and mobile devices
- **Detailed Explanations**: Line-by-line commentary of how the code works
- **Complete API Reference**: Full documentation of all functions and data structures
- **Interactive 3D Game**: Three.js powered game where users can control philosophers' actions
- **Educational Content**: Lessons about concurrency concepts built into the interactive elements
