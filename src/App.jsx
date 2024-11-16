import "./App.css";
import { useState } from "react";
import ProgressBar from "./components/ProgressBar";
import MatchaPic from "./assets/matchapic.png";

function App() {
  const [loc, setLoc] = useState("1"); // Initial value
  const [activeBar, setActiveBar] = useState(null); // Track which bar is active

  const startTimer = (barId) => {
    setActiveBar(barId); // Set the active progress bar
  };

  const handleSelect = (e) => {
    setLoc(e.target.value);
    console.log(loc);
  };

  return (
    <div>
      <h1>MaTTcha Whisker</h1>
      <label>Select a cup location: </label>
      <select onChange={handleSelect}>
        <option value="1">1</option>
        <option value="2">2</option>
      </select>
      <br></br>
      <button
        onClick={() => startTimer(`bar${loc}`)} // Start progress bar based on selected location
        disabled={activeBar === `bar${loc}`} // Disable button if selected bar is active
        style={{ marginTop: "10px" }}
      >
        Go!
      </button>
      <div className="progress-bars">
        <ProgressBar
          id="1"
          isActive={activeBar === "bar1"}
          startTimer={() => startTimer("bar1")}
        />
        <ProgressBar
          id="2"
          isActive={activeBar === "bar2"}
          startTimer={() => startTimer("bar2")}
        />
      </div>
      <img
        src={MatchaPic}
        alt="Matcha"
        style={{ maxWidth: "100%", maxHeight: "300px", marginTop: "2%" }}
      />
    </div>
  );
}

export default App;
