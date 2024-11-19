import React, { useState } from "react";
import ProgressBar from "./components/ProgressBar";
import "./App.css";

function App() {
  const [loc, setLoc] = useState("1"); // Initial value
  const [activeBar, setActiveBar] = useState(null); // Track which bar is active
  const [isWhisking, setIsWhisking] = useState(false); // Track if whisking is happening

  const handleSelect = (e) => {
    setLoc(e.target.value);
  };

  const startWhisking = async () => {
    if (!isWhisking) {
      setIsWhisking(true); // Start the whisking process
      try {
        const response = await fetch("http://192.168.4.1/start"); // Make sure this matches your Arduino AP IP
        if (response.ok) {
          console.log("Whisking started");
          setActiveBar(`bar${loc}`);
          setTimeout(() => {
            setIsWhisking(false);
          }, 60000); // Stop whisking after 1 minute
        } else {
          console.error("Failed to start whisking");
        }
      } catch (error) {
        console.error("Error connecting to Arduino:", error);
      }
    }
  };

  return (
    <div>
      <h1>MaTTcha Whisker</h1>
      <label>Select a cup location: </label>
      <select onChange={handleSelect}>
        <option value="1">1</option>
        <option value="2">2</option>
      </select>
      <br />
      <button onClick={startWhisking} disabled={isWhisking}>
        {isWhisking ? "Whisking..." : "Go!"}
      </button>
      <div className="progress-bars">
        <ProgressBar id="1" isActive={activeBar === "bar1"} />
        <ProgressBar id="2" isActive={activeBar === "bar2"} />
      </div>
    </div>
  );
}

export default App;