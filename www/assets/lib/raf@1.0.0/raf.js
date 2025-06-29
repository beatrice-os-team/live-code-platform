if (typeof window.RAFManager === 'undefined') {
    window.RAFManager = {
        callbacks: [],
        running: false,

        add(callback) {
            this.callbacks.push(callback);
            if (!this.running) {
                this.start();
            }
        },

        start() {
            const loop = () => {
                for (let cb of this.callbacks) {
                    cb();
                }
                window.requestAnimationFrame(loop);
            };
            this.running = true;
            window.requestAnimationFrame(loop);
        }
    };
    console.log('RAFManager init');
} else {
    console.log('RAFManager already init');
}