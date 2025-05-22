// Code highlighting effects for connecting concepts
document.addEventListener('DOMContentLoaded', function() {
    // Initialize highlighting system
    initHighlighting();
    
    // Add event listeners to highlight triggers
    setupHighlightTriggers();
});

// Initialize code highlighting effect
function initHighlighting() {
    // Add pulsing effect to highlighted code sections
    const style = document.createElement('style');
    style.textContent = `
        @keyframes pulse-highlight {
            0% { background-color: rgba(126, 87, 194, 0.1); }
            50% { background-color: rgba(126, 87, 194, 0.3); }
            100% { background-color: rgba(126, 87, 194, 0.1); }
        }
        
        .highlight-pulse {
            animation: pulse-highlight 2s infinite;
            border-radius: 3px;
        }
        
        .highlight-trigger {
            cursor: pointer;
            text-decoration: underline;
            color: #7e57c2;
        }
        
        .highlight-trigger:hover {
            color: #5e35b1;
            text-decoration: none;
        }
    `;
    document.head.appendChild(style);
}

// Set up highlight trigger elements
function setupHighlightTriggers() {
    const triggers = document.querySelectorAll('.highlight-trigger');
    
    triggers.forEach(trigger => {
        trigger.addEventListener('click', function() {
            // Get target code element ID
            const targetId = this.dataset.target;
            const targetElement = document.getElementById(targetId);
            
            if (targetElement) {
                // Remove any existing highlights
                const existingHighlights = document.querySelectorAll('.highlight-pulse');
                existingHighlights.forEach(el => el.classList.remove('highlight-pulse'));
                
                // Add highlight to target
                targetElement.classList.add('highlight-pulse');
                
                // Scroll to element if not in view
                targetElement.scrollIntoView({
                    behavior: 'smooth',
                    block: 'center'
                });
                
                // Remove highlight after 5 seconds
                setTimeout(() => {
                    targetElement.classList.remove('highlight-pulse');
                }, 5000);
            }
        });
    });
}

// Function to highlight a specific code section
function highlightCode(targetId) {
    const targetElement = document.getElementById(targetId);
    
    if (targetElement) {
        // Remove any existing highlights
        const existingHighlights = document.querySelectorAll('.highlight-pulse');
        existingHighlights.forEach(el => el.classList.remove('highlight-pulse'));
        
        // Add highlight to target
        targetElement.classList.add('highlight-pulse');
        
        // Scroll to element if not in view
        targetElement.scrollIntoView({
            behavior: 'smooth',
            block: 'center'
        });
        
        // Remove highlight after 5 seconds
        setTimeout(() => {
            targetElement.classList.remove('highlight-pulse');
        }, 5000);
    }
}
