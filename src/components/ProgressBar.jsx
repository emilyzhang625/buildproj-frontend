import React, { useState, useEffect } from "react";

const TimerProgressBar = ({ id, isActive, startTimer }) => {
  const [time, setTime] = useState(0); // Start with 0
  const [isFinished, setIsFinished] = useState(false); // Track if finished

  useEffect(() => {
    let interval;

    // Timer runs only when active and not finished
    if (isActive && time < 60 && !isFinished) {
      interval = setInterval(() => {
        setTime((prevTime) => prevTime + 1);
      }, 1000);
    }

    // When timer reaches 60, stop and mark as finished
    if (time >= 60) {
      clearInterval(interval); // Stop the interval
      setIsFinished(true); // Mark the timer as finished
    }

    return () => clearInterval(interval); // Clean up interval on unmount or when dependencies change
  }, [isActive, time, isFinished]);

  const progress = (time / 60) * 100;

  const containerStyles = {
    height: "30px",
    width: "100%",
    backgroundColor: "#e0e0df",
    borderRadius: "5px",
    overflow: "hidden",
  };

  const fillerStyles = {
    height: "100%",
    width: `${progress}%`,
    backgroundColor: "#4d6e42",
    transition: "width 1s linear",
  };

  const labelStyles = {
    padding: "5px",
    color: "white",
    fontWeight: "bold",
  };

  const handleStart = () => {
    setIsFinished(false); // Reset finished state when starting
    setTime(0); // Reset time to 0
    startTimer(id); // Start the timer (notify parent)
  };

  return (
    <div>
      <h2>{`Location ${id}`}</h2>
      <div style={containerStyles}>
        <div style={fillerStyles}>
          <span style={labelStyles}>{`${time}s`}</span>
        </div>
      </div>
      {isFinished && (
        <div style={{ marginTop: "10px", color: "#4d6e42" }}>Finished!</div>
      )}
      {/* Only show the "Start" button when the timer is finished */}
      {isFinished && <button onClick={handleStart}>Start Again</button>}
    </div>
  );
};

export default TimerProgressBar;
