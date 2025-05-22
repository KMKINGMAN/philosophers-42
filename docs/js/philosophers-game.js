// Philosophers Game - Interactive 3D simulation using Three.js

// Game state variables
let gameRunning = false;
let gameScore = 0;
let gameTime = 0;
let selectedPhilosopher = null;
let philosophers = [];
let forks = [];
let gameInterval = null;

// Three.js variables
let scene, camera, renderer, controls;
let table, room;
let raycaster, mouse;

// Constants
const NUM_PHILOSOPHERS = 5;
const TABLE_RADIUS = 5;
const FORK_SIZE = 0.8;
const PHILOSOPHER_SIZE = 1;
const PHILOSOPHER_COLORS = [
    0xff4444, // Red
    0x44ff44, // Green
    0x4444ff, // Blue
    0xffff44, // Yellow
    0xff44ff  // Purple
];

// Philosopher states
const STATES = {
    THINKING: 'thinking',
    HUNGRY: 'hungry',
    EATING: 'eating'
};

// Initialize the game
function initGame() {
    // Reset game state
    gameRunning = false;
    gameScore = 0;
    gameTime = 0;
    selectedPhilosopher = null;
    philosophers = [];
    forks = [];
    
    if (gameInterval) {
        clearInterval(gameInterval);
        gameInterval = null;
    }
    
    // Update UI
    updateUI();
    
    // Initialize Three.js scene
    initThreeJS();
    
    // Create philosophers and forks
    createPhilosophers();
    createForks();
    createTable();
    createRoom();
    
    // Start animation loop
    animate();
}

// Initialize Three.js
function initThreeJS() {
    // Create scene
    scene = new THREE.Scene();
    scene.background = new THREE.Color(0x222233);
    
    // Create camera
    camera = new THREE.PerspectiveCamera(75, 
        document.getElementById('game-container').offsetWidth / 
        document.getElementById('game-container').offsetHeight, 0.1, 1000);
    camera.position.set(0, 12, 12);
    camera.lookAt(0, 0, 0);
    
    // Create renderer
    renderer = new THREE.WebGLRenderer({ 
        canvas: document.getElementById('game-canvas'),
        antialias: true
    });
    renderer.setSize(document.getElementById('game-container').offsetWidth, 
                    document.getElementById('game-container').offsetHeight);
    renderer.shadowMap.enabled = true;
    
    // Add lights
    const ambientLight = new THREE.AmbientLight(0x404040, 0.5);
    scene.add(ambientLight);
    
    const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
    directionalLight.position.set(10, 20, 10);
    directionalLight.castShadow = true;
    scene.add(directionalLight);
    
    const pointLight = new THREE.PointLight(0xffffff, 0.5);
    pointLight.position.set(0, 10, 0);
    scene.add(pointLight);
    
    // Setup raycaster for mouse interaction
    raycaster = new THREE.Raycaster();
    mouse = new THREE.Vector2();
    
    // Add event listeners
    window.addEventListener('resize', onWindowResize);
    document.getElementById('game-canvas').addEventListener('click', onMouseClick);
}

// Create table
function createTable() {
    const tableGeometry = new THREE.CylinderGeometry(TABLE_RADIUS, TABLE_RADIUS, 0.5, 32);
    const tableMaterial = new THREE.MeshPhongMaterial({ 
        color: 0x8B4513,
        shininess: 30
    });
    table = new THREE.Mesh(tableGeometry, tableMaterial);
    table.position.y = -0.25;
    table.receiveShadow = true;
    scene.add(table);
    
    // Add tablecloth texture
    const clothGeometry = new THREE.CylinderGeometry(TABLE_RADIUS - 0.1, TABLE_RADIUS - 0.1, 0.05, 32);
    const clothMaterial = new THREE.MeshPhongMaterial({ 
        color: 0x228B22,
        shininess: 10
    });
    const tablecloth = new THREE.Mesh(clothGeometry, clothMaterial);
    tablecloth.position.y = 0.15;
    tablecloth.receiveShadow = true;
    scene.add(tablecloth);
    
    // Add central bowl of spaghetti
    const bowlGeometry = new THREE.CylinderGeometry(1.5, 1, 0.7, 32);
    const bowlMaterial = new THREE.MeshPhongMaterial({ 
        color: 0xFFFFFF,
        shininess: 100
    });
    const bowl = new THREE.Mesh(bowlGeometry, bowlMaterial);
    bowl.position.y = 0.55;
    bowl.castShadow = true;
    bowl.receiveShadow = true;
    scene.add(bowl);
    
    // Add spaghetti
    const spaghettiGeometry = new THREE.SphereGeometry(1.2, 32, 32);
    const spaghettiMaterial = new THREE.MeshPhongMaterial({ 
        color: 0xFFD700,
        shininess: 30
    });
    const spaghetti = new THREE.Mesh(spaghettiGeometry, spaghettiMaterial);
    spaghetti.position.y = 0.8;
    spaghetti.scale.y = 0.3;
    spaghetti.castShadow = true;
    scene.add(spaghetti);
}

// Create room environment
function createRoom() {
    const roomSize = 30;
    const roomGeometry = new THREE.BoxGeometry(roomSize, roomSize, roomSize);
    const roomMaterials = [
        new THREE.MeshPhongMaterial({ color: 0x8B4513, side: THREE.BackSide }), // right
        new THREE.MeshPhongMaterial({ color: 0x8B4513, side: THREE.BackSide }), // left
        new THREE.MeshPhongMaterial({ color: 0xCCCCCC, side: THREE.BackSide }), // top
        new THREE.MeshPhongMaterial({ color: 0x774433, side: THREE.BackSide }), // bottom
        new THREE.MeshPhongMaterial({ color: 0x8B4513, side: THREE.BackSide }), // front
        new THREE.MeshPhongMaterial({ color: 0x8B4513, side: THREE.BackSide })  // back
    ];
    
    room = new THREE.Mesh(roomGeometry, roomMaterials);
    scene.add(room);
    
    // Add a simple chandelier
    const chandelierGeometry = new THREE.SphereGeometry(0.5, 16, 16);
    const chandelierMaterial = new THREE.MeshPhongMaterial({ 
        color: 0xFFD700,
        emissive: 0xFFD700,
        emissiveIntensity: 0.5
    });
    const chandelier = new THREE.Mesh(chandelierGeometry, chandelierMaterial);
    chandelier.position.y = 10;
    scene.add(chandelier);
    
    // Add a point light to the chandelier
    const chandelierLight = new THREE.PointLight(0xFFFFDD, 1, 30);
    chandelierLight.position.copy(chandelier.position);
    scene.add(chandelierLight);
}

// Create philosophers
function createPhilosophers() {
    const stats = document.getElementById('philosopher-stats');
    stats.innerHTML = ''; // Clear existing stats
    
    for (let i = 0; i < NUM_PHILOSOPHERS; i++) {
        // Calculate position around the table
        const angle = (i / NUM_PHILOSOPHERS) * Math.PI * 2;
        const x = Math.cos(angle) * (TABLE_RADIUS - 1.5);
        const z = Math.sin(angle) * (TABLE_RADIUS - 1.5);
        
        // Create philosopher object
        const philosopherGeometry = new THREE.CylinderGeometry(0, PHILOSOPHER_SIZE / 2, PHILOSOPHER_SIZE, 8);
        const philosopherMaterial = new THREE.MeshPhongMaterial({ 
            color: PHILOSOPHER_COLORS[i],
            shininess: 30
        });
        const philosopherMesh = new THREE.Mesh(philosopherGeometry, philosopherMaterial);
        philosopherMesh.position.set(x, 0.5, z);
        philosopherMesh.rotation.x = Math.PI; // Flip to make pointy end up
        philosopherMesh.castShadow = true;
        philosopherMesh.userData.isPhilosopher = true;
        philosopherMesh.userData.index = i;
        scene.add(philosopherMesh);
        
        // Add head
        const headGeometry = new THREE.SphereGeometry(PHILOSOPHER_SIZE / 3, 32, 32);
        const headMaterial = new THREE.MeshPhongMaterial({ 
            color: 0xFFD3B5,
            shininess: 30
        });
        const head = new THREE.Mesh(headGeometry, headMaterial);
        head.position.y = 0.7;
        philosopherMesh.add(head);
        
        // Create philosopher data
        const philosopher = {
            mesh: philosopherMesh,
            id: i + 1,
            state: STATES.THINKING,
            hunger: 0,
            mealsEaten: 0,
            leftForkId: i,
            rightForkId: (i + 1) % NUM_PHILOSOPHERS,
            hasLeftFork: false,
            hasRightFork: false,
            angle: angle,
            highlight: new THREE.Mesh(
                new THREE.RingGeometry(PHILOSOPHER_SIZE / 2 + 0.2, PHILOSOPHER_SIZE / 2 + 0.4, 32),
                new THREE.MeshBasicMaterial({ color: 0xFFFFFF, side: THREE.DoubleSide })
            )
        };
        
        // Add highlight ring
        philosopher.highlight.rotation.x = Math.PI / 2;
        philosopher.highlight.position.y = 0.05;
        philosopher.highlight.visible = false;
        philosopherMesh.add(philosopher.highlight);
        
        philosophers.push(philosopher);
        
        // Create stat element
        const statDiv = document.createElement('div');
        statDiv.className = 'philo-stat';
        statDiv.style.backgroundColor = '#' + PHILOSOPHER_COLORS[i].toString(16);
        statDiv.innerHTML = `<div>P${i+1}</div><div class="hunger-bar" style="width:100px; height:10px; background:#444;"><div class="hunger-fill" style="width:0%; height:100%; background:red;"></div></div>`;
        stats.appendChild(statDiv);
    }
}

// Create forks
function createForks() {
    for (let i = 0; i < NUM_PHILOSOPHERS; i++) {
        // Calculate position between philosophers
        const angle = ((i + 0.5) / NUM_PHILOSOPHERS) * Math.PI * 2;
        const x = Math.cos(angle) * (TABLE_RADIUS - 2.2);
        const z = Math.sin(angle) * (TABLE_RADIUS - 2.2);
        
        // Create fork geometry
        const forkGeometry = new THREE.BoxGeometry(FORK_SIZE, 0.05, FORK_SIZE / 5);
        const forkMaterial = new THREE.MeshPhongMaterial({ 
            color: 0xC0C0C0,
            metalness: 0.7,
            shininess: 100
        });
        const forkMesh = new THREE.Mesh(forkGeometry, forkMaterial);
        forkMesh.position.set(x, 0.15, z);
        forkMesh.rotation.y = angle + Math.PI / 2;
        forkMesh.castShadow = true;
        forkMesh.userData.isFork = true;
        forkMesh.userData.index = i;
        scene.add(forkMesh);
        
        // Create tines
        const tineGeometry = new THREE.BoxGeometry(FORK_SIZE / 6, 0.05, FORK_SIZE / 2);
        const tineMaterial = new THREE.MeshPhongMaterial({ 
            color: 0xC0C0C0,
            metalness: 0.7,
            shininess: 100
        });
        
        for (let j = 0; j < 3; j++) {
            const tine = new THREE.Mesh(tineGeometry, tineMaterial);
            tine.position.z = FORK_SIZE / 3;
            tine.position.x = (j - 1) * (FORK_SIZE / 5);
            forkMesh.add(tine);
        }
        
        // Create fork data
        const fork = {
            mesh: forkMesh,
            id: i,
            inUse: false,
            heldBy: null,
            originalPosition: new THREE.Vector3(x, 0.15, z),
            originalRotation: forkMesh.rotation.clone()
        };
        
        forks.push(fork);
    }
}

// Game loop
function startGame() {
    gameRunning = true;
    
    // Reset game state
    gameScore = 0;
    gameTime = 0;
    
    // Ensure philosophers start in thinking state
    philosophers.forEach(philosopher => {
        philosopher.state = STATES.THINKING;
        philosopher.hunger = 0;
        philosopher.mealsEaten = 0;
        philosopher.hasLeftFork = false;
        philosopher.hasRightFork = false;
    });
    
    // Reset forks
    forks.forEach(fork => {
        fork.inUse = false;
        fork.heldBy = null;
        fork.mesh.position.copy(fork.originalPosition);
        fork.mesh.rotation.copy(fork.originalRotation);
    });
    
    // Update UI
    updateUI();
    
    // Start game timer
    gameInterval = setInterval(gameUpdate, 100);
}

// Game update (called every 100ms)
function gameUpdate() {
    // Increase game time
    gameTime += 0.1;
    
    // Update philosophers
    philosophers.forEach(philosopher => {
        // Increase hunger for non-eating philosophers
        if (philosopher.state !== STATES.EATING) {
            philosopher.hunger += 0.2;
            
            // Cap hunger at 100
            if (philosopher.hunger > 100) {
                philosopher.hunger = 100;
                endGame(false, `Philosopher ${philosopher.id} has starved!`);
            }
        } else {
            // Decrease hunger when eating
            philosopher.hunger -= 0.5;
            if (philosopher.hunger < 0) philosopher.hunger = 0;
        }
    });
    
    // Update UI
    updateUI();
    
    // Check if all philosophers have eaten enough
    let allEaten = philosophers.every(p => p.mealsEaten >= 3);
    if (allEaten) {
        gameScore += 10;
        // Reset meal counts for another round
        philosophers.forEach(p => p.mealsEaten = 0);
        M.toast({html: 'All philosophers have eaten successfully! +10 points', classes: 'green'});
    }
}

// End the game
function endGame(win, message) {
    gameRunning = false;
    clearInterval(gameInterval);
    
    if (win) {
        M.toast({html: `${message} Final score: ${gameScore}`, classes: 'green'});
    } else {
        M.toast({html: `${message} Game over! Final score: ${gameScore}`, classes: 'red'});
    }
    
    // Disable action buttons
    enableActionButtons(false);
}

// Reset the game
function resetGame() {
    if (gameInterval) {
        clearInterval(gameInterval);
    }
    
    initGame();
    M.toast({html: 'Game reset', classes: 'blue'});
}

// Update UI elements
function updateUI() {
    // Update score and time
    document.getElementById('score-display').textContent = `Score: ${gameScore}`;
    document.getElementById('time-display').textContent = `Time: ${gameTime.toFixed(1)}s`;
    
    // Update selected philosopher display
    if (selectedPhilosopher !== null) {
        document.getElementById('selected-philosopher').textContent = `Philosopher ${philosophers[selectedPhilosopher].id}`;
        
        // Enable action buttons based on state
        enableActionButtons(true);
        updateActionButtons();
    } else {
        document.getElementById('selected-philosopher').textContent = 'None';
        enableActionButtons(false);
    }
    
    // Update philosopher stats
    philosophers.forEach((philosopher, i) => {
        const statDiv = document.getElementById('philosopher-stats').children[i];
        const hungerBar = statDiv.querySelector('.hunger-fill');
        hungerBar.style.width = `${philosopher.hunger}%`;
        
        // Change color based on hunger
        if (philosopher.hunger < 30) {
            hungerBar.style.backgroundColor = '#4CAF50'; // Green
        } else if (philosopher.hunger < 70) {
            hungerBar.style.backgroundColor = '#FF9800'; // Orange
        } else {
            hungerBar.style.backgroundColor = '#F44336'; // Red
        }
        
        // Update status text
        statDiv.querySelector('div').textContent = `P${philosopher.id}: ${philosopher.state.charAt(0).toUpperCase() + philosopher.state.slice(1)} (Meals: ${philosopher.mealsEaten})`;
        
        // Update mesh color based on state
        switch (philosopher.state) {
            case STATES.THINKING:
                philosopher.mesh.material.color.setHex(PHILOSOPHER_COLORS[i]);
                break;
            case STATES.HUNGRY:
                philosopher.mesh.material.color.setHex(0xFF9800); // Orange
                break;
            case STATES.EATING:
                philosopher.mesh.material.color.setHex(0x4CAF50); // Green
                break;
        }
    });
}

// Enable/disable action buttons
function enableActionButtons(enabled) {
    const buttons = [
        document.getElementById('action-think'),
        document.getElementById('action-take-forks'),
        document.getElementById('action-eat'),
        document.getElementById('action-release-forks')
    ];
    
    buttons.forEach(button => {
        button.disabled = !enabled;
    });
}

// Update action buttons based on philosopher state
function updateActionButtons() {
    if (selectedPhilosopher === null) return;
    
    const philosopher = philosophers[selectedPhilosopher];
    const takeForksButton = document.getElementById('action-take-forks');
    const eatButton = document.getElementById('action-eat');
    const releaseForksButton = document.getElementById('action-release-forks');
    
    // Enable/disable based on current state and fork availability
    takeForksButton.disabled = philosopher.hasLeftFork && philosopher.hasRightFork;
    eatButton.disabled = !(philosopher.hasLeftFork && philosopher.hasRightFork);
    releaseForksButton.disabled = !philosopher.hasLeftFork && !philosopher.hasRightFork;
}

// Philosopher actions
function philosopherThink(philosopherId) {
    const philosopher = philosophers[philosopherId];
    philosopher.state = STATES.THINKING;
    updateUI();
}

function philosopherTakeForks(philosopherId) {
    const philosopher = philosophers[philosopherId];
    philosopher.state = STATES.HUNGRY;
    
    // Try to take left fork
    if (!philosopher.hasLeftFork) {
        const leftFork = forks[philosopher.leftForkId];
        if (!leftFork.inUse) {
            leftFork.inUse = true;
            leftFork.heldBy = philosopherId;
            philosopher.hasLeftFork = true;
            
            // Move fork to philosopher's position
            moveForkToPhilosopher(leftFork, philosopher, true);
        }
    }
    
    // Try to take right fork
    if (!philosopher.hasRightFork) {
        const rightFork = forks[philosopher.rightForkId];
        if (!rightFork.inUse) {
            rightFork.inUse = true;
            rightFork.heldBy = philosopherId;
            philosopher.hasRightFork = true;
            
            // Move fork to philosopher's position
            moveForkToPhilosopher(rightFork, philosopher, false);
        }
    }
    
    updateUI();
    updateActionButtons();
}

function philosopherEat(philosopherId) {
    const philosopher = philosophers[philosopherId];
    
    // Can only eat if has both forks
    if (philosopher.hasLeftFork && philosopher.hasRightFork) {
        philosopher.state = STATES.EATING;
        philosopher.mealsEaten++;
        gameScore++;
        
        updateUI();
    }
}

function philosopherReleaseForks(philosopherId) {
    const philosopher = philosophers[philosopherId];
    
    // Release left fork
    if (philosopher.hasLeftFork) {
        const leftFork = forks[philosopher.leftForkId];
        leftFork.inUse = false;
        leftFork.heldBy = null;
        philosopher.hasLeftFork = false;
        
        // Move fork back to original position
        leftFork.mesh.position.copy(leftFork.originalPosition);
        leftFork.mesh.rotation.copy(leftFork.originalRotation);
    }
    
    // Release right fork
    if (philosopher.hasRightFork) {
        const rightFork = forks[philosopher.rightForkId];
        rightFork.inUse = false;
        rightFork.heldBy = null;
        philosopher.hasRightFork = false;
        
        // Move fork back to original position
        rightFork.mesh.position.copy(rightFork.originalPosition);
        rightFork.mesh.rotation.copy(rightFork.originalRotation);
    }
    
    updateUI();
    updateActionButtons();
}

// Move fork to philosopher
function moveForkToPhilosopher(fork, philosopher, isLeft) {
    // Calculate offset based on whether it's left or right fork
    const angleOffset = isLeft ? -0.3 : 0.3;
    const angle = philosopher.angle + angleOffset;
    
    // Position near philosopher's hand
    const x = Math.cos(philosopher.angle) * (TABLE_RADIUS - 1.5) + Math.cos(angle) * 0.5;
    const z = Math.sin(philosopher.angle) * (TABLE_RADIUS - 1.5) + Math.sin(angle) * 0.5;
    
    fork.mesh.position.set(x, 0.5, z);
    fork.mesh.rotation.y = philosopher.angle + Math.PI / 2;
}

// Event handlers
function onMouseClick(event) {
    // Calculate mouse position in normalized device coordinates
    const rect = renderer.domElement.getBoundingClientRect();
    mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;
    
    // Update the picking ray with the camera and mouse position
    raycaster.setFromCamera(mouse, camera);
    
    // Calculate objects intersecting the picking ray
    const intersects = raycaster.intersectObjects(scene.children, true);
    
    // Check if we hit a philosopher
    for (let i = 0; i < intersects.length; i++) {
        const object = intersects[i].object;
        
        // Walk up the parent chain to find the philosopher
        let current = object;
        while (current && !current.userData.isPhilosopher) {
            current = current.parent;
        }
        
        if (current && current.userData.isPhilosopher) {
            selectPhilosopher(current.userData.index);
            return;
        }
    }
    
    // If we didn't hit a philosopher, deselect
    deselectPhilosopher();
}

// Select a philosopher
function selectPhilosopher(index) {
    // Deselect previous philosopher if any
    if (selectedPhilosopher !== null) {
        philosophers[selectedPhilosopher].highlight.visible = false;
    }
    
    // Select new philosopher
    selectedPhilosopher = index;
    philosophers[selectedPhilosopher].highlight.visible = true;
    
    // Update UI
    updateUI();
    
    // Play select sound
    // playSound('select');
}

// Deselect philosopher
function deselectPhilosopher() {
    if (selectedPhilosopher !== null) {
        philosophers[selectedPhilosopher].highlight.visible = false;
        selectedPhilosopher = null;
    }
    
    // Update UI
    updateUI();
}

// Window resize handler
function onWindowResize() {
    camera.aspect = document.getElementById('game-container').offsetWidth / 
                    document.getElementById('game-container').offsetHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(document.getElementById('game-container').offsetWidth, 
                    document.getElementById('game-container').offsetHeight);
}

// Animation loop
function animate() {
    requestAnimationFrame(animate);
    
    // Rotate camera slightly
    const time = Date.now() * 0.0005;
    const cameraRadius = 15;
    camera.position.x = Math.sin(time * 0.1) * cameraRadius;
    camera.position.z = Math.cos(time * 0.1) * cameraRadius;
    camera.position.y = 10 + Math.sin(time * 0.05) * 2;
    camera.lookAt(0, 0, 0);
    
    renderer.render(scene, camera);
}

// Setup event listeners for UI
document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('start-game').addEventListener('click', function() {
        if (!gameRunning) {
            startGame();
            this.textContent = 'Pause Game';
            this.innerHTML = 'Pause Game <i class="material-icons right">pause</i>';
        } else {
            gameRunning = false;
            clearInterval(gameInterval);
            this.textContent = 'Resume Game';
            this.innerHTML = 'Resume Game <i class="material-icons right">play_arrow</i>';
        }
    });
    
    document.getElementById('reset-game').addEventListener('click', resetGame);
    
    // Action buttons
    document.getElementById('action-think').addEventListener('click', function() {
        if (selectedPhilosopher !== null) {
            philosopherThink(selectedPhilosopher);
        }
    });
    
    document.getElementById('action-take-forks').addEventListener('click', function() {
        if (selectedPhilosopher !== null) {
            philosopherTakeForks(selectedPhilosopher);
        }
    });
    
    document.getElementById('action-eat').addEventListener('click', function() {
        if (selectedPhilosopher !== null) {
            philosopherEat(selectedPhilosopher);
        }
    });
    
    document.getElementById('action-release-forks').addEventListener('click', function() {
        if (selectedPhilosopher !== null) {
            philosopherReleaseForks(selectedPhilosopher);
        }
    });
    
    // Initialize the game
    initGame();
});
