// Main JavaScript for Philosophers documentation

document.addEventListener('DOMContentLoaded', function() {
    // Initialize Materialize components
    M.AutoInit();
    
    // Initialize parallax
    var elems = document.querySelectorAll('.parallax');
    var instances = M.Parallax.init(elems);
    
    // Initialize tabs
    var tabs = document.querySelectorAll('.tabs');
    var tabsInstances = M.Tabs.init(tabs);
    
    // Initialize collapsibles
    var collapsibles = document.querySelectorAll('.collapsible');
    var collapsiblesInstances = M.Collapsible.init(collapsibles);
    
    // Initialize scrollspy
    var scrollspy = document.querySelectorAll('.scrollspy');
    var scrollspyInstances = M.ScrollSpy.init(scrollspy, {
        scrollOffset: 100
    });
    
    // Initialize tooltips
    var tooltips = document.querySelectorAll('.tooltipped');
    var tooltipsInstances = M.Tooltip.init(tooltips);
    
    // Initialize modals
    var modals = document.querySelectorAll('.modal');
    var modalInstances = M.Modal.init(modals);
    
    // Check if we're on the visualization page
    if (document.getElementById('visualization-canvas')) {
        initVisualization();
    }
});

// Function to initialize the visualization using PixiJS
function initVisualization() {
    // Create a PixiJS Application
    const app = new PIXI.Application({
        width: document.getElementById('visualization-canvas').offsetWidth,
        height: 500,
        backgroundColor: 0xEEEEEE,
        resolution: window.devicePixelRatio || 1,
        autoDensity: true,
        antialias: true
    });
    
    // Add the Pixi Canvas to the HTML
    document.getElementById('visualization-canvas').appendChild(app.view);
    
    // Table setup
    const table = new PIXI.Graphics();
    table.beginFill(0x8D6E63);
    table.drawCircle(app.screen.width / 2, app.screen.height / 2, 150);
    table.endFill();
    app.stage.addChild(table);
    
    // Default setup variables
    let numPhilosophers = 5;
    let philosophers = [];
    let forks = [];
    let simulationRunning = false;
    let speedMultiplier = 1;
    
    // Button events
    document.getElementById('start-simulation').addEventListener('click', function() {
        if (!simulationRunning) {
            simulationRunning = true;
            startSimulation();
            this.textContent = 'Pause Simulation';
        } else {
            simulationRunning = false;
            this.textContent = 'Resume Simulation';
        }
    });
    
    document.getElementById('reset-simulation').addEventListener('click', function() {
        simulationRunning = false;
        document.getElementById('start-simulation').textContent = 'Start Simulation';
        resetSimulation();
    });
    
    document.getElementById('num-philosophers').addEventListener('change', function() {
        numPhilosophers = parseInt(this.value);
        resetSimulation();
    });
    
    document.getElementById('simulation-speed').addEventListener('change', function() {
        speedMultiplier = parseFloat(this.value);
    });
    
    // Setup initial visualization
    function setupVisualization() {
        // Clear any existing elements
        while(philosophers.length > 0) {
            app.stage.removeChild(philosophers.pop());
        }
        
        while(forks.length > 0) {
            app.stage.removeChild(forks.pop());
        }
        
        const centerX = app.screen.width / 2;
        const centerY = app.screen.height / 2;
        const radius = 180;
        
        // Create philosophers
        for (let i = 0; i < numPhilosophers; i++) {
            const angle = (i / numPhilosophers) * Math.PI * 2;
            const x = centerX + Math.cos(angle) * radius;
            const y = centerY + Math.sin(angle) * radius;
            
            // Create philosopher sprite
            const philosopher = new PIXI.Graphics();
            philosopher.beginFill(0x9C27B0);
            philosopher.drawCircle(0, 0, 25);
            philosopher.endFill();
            
            // Add philosopher ID text
            const text = new PIXI.Text(String(i + 1), {
                fontFamily: 'Arial',
                fontSize: 16,
                fill: 'white',
                align: 'center'
            });
            text.anchor.set(0.5);
            philosopher.addChild(text);
            
            philosopher.x = x;
            philosopher.y = y;
            philosopher.id = i + 1;
            philosopher.status = 'thinking';  // Initial state
            philosopher.interactive = true;
            philosopher.buttonMode = true;
            
            philosopher.on('pointerdown', function() {
                showPhilosopherInfo(this.id, this.status);
            });
            
            app.stage.addChild(philosopher);
            philosophers.push(philosopher);
            
            // Create fork for this philosopher
            const forkAngle = angle + (Math.PI / numPhilosophers);
            const forkX = centerX + Math.cos(forkAngle) * (radius - 40);
            const forkY = centerY + Math.sin(forkAngle) * (radius - 40);
            
            const fork = new PIXI.Graphics();
            fork.lineStyle(3, 0xCCCCCC);
            fork.moveTo(-5, -10);
            fork.lineTo(5, 10);
            fork.moveTo(-5, 10);
            fork.lineTo(5, -10);
            
            fork.x = forkX;
            fork.y = forkY;
            fork.id = i + 1;
            fork.inUse = false;
            
            app.stage.addChild(fork);
            forks.push(fork);
        }
        
        // Update initial status display
        updateStatusDisplay();
    }
    
    function startSimulation() {
        if (!simulationRunning) return;
        
        // Handle actions for each philosopher
        for (let i = 0; i < philosophers.length; i++) {
            const philosopher = philosophers[i];
            
            // Randomly change philosopher state based on current state and fork availability
            if (philosopher.status === 'thinking') {
                // Philosopher tries to eat
                const leftForkIndex = i;
                const rightForkIndex = (i + 1) % numPhilosophers;
                
                if (!forks[leftForkIndex].inUse && !forks[rightForkIndex].inUse) {
                    // Can grab both forks
                    forks[leftForkIndex].inUse = true;
                    forks[rightForkIndex].inUse = true;
                    philosopher.status = 'eating';
                    philosopher.tint = 0x4CAF50; // Green
                    
                    // Visually show forks in use
                    forks[leftForkIndex].tint = 0xFFD700;  // Gold
                    forks[rightForkIndex].tint = 0xFFD700;  // Gold
                    
                    // Set eating timeout
                    setTimeout(() => {
                        if (simulationRunning && philosopher.status === 'eating') {
                            philosopher.status = 'sleeping';
                            philosopher.tint = 0xFF9800; // Orange
                            forks[leftForkIndex].inUse = false;
                            forks[rightForkIndex].inUse = false;
                            forks[leftForkIndex].tint = 0xFFFFFF; // Reset color
                            forks[rightForkIndex].tint = 0xFFFFFF; // Reset color
                        }
                    }, 3000 / speedMultiplier);
                }
            } else if (philosopher.status === 'sleeping') {
                // Set sleep timeout
                setTimeout(() => {
                    if (simulationRunning && philosopher.status === 'sleeping') {
                        philosopher.status = 'thinking';
                        philosopher.tint = 0x2196F3; // Blue
                    }
                }, 2000 / speedMultiplier);
            }
        }
        
        // Update the status display
        updateStatusDisplay();
        
        // Schedule the next animation frame
        setTimeout(() => {
            if (simulationRunning) {
                startSimulation();
            }
        }, 1000 / speedMultiplier);
    }
    
    function resetSimulation() {
        setupVisualization();
    }
    
    function updateStatusDisplay() {
        const statusContainer = document.getElementById('status-container');
        statusContainer.innerHTML = '';
        
        for (let i = 0; i < philosophers.length; i++) {
            const philosopher = philosophers[i];
            const statusDiv = document.createElement('div');
            statusDiv.className = `philosopher-status status-${philosopher.status}`;
            statusDiv.textContent = `Philosopher ${philosopher.id}: ${philosopher.status.toUpperCase()}`;
            statusContainer.appendChild(statusDiv);
        }
    }
    
    function showPhilosopherInfo(id, status) {
        M.toast({html: `Philosopher ${id} is currently ${status}`, classes: 'rounded'});
    }
    
    // Initialize the visualization
    setupVisualization();
}
