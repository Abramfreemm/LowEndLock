"""Python reference for Low-End Lock phase/polarity analysis.

This script validates the core idea before translating it to C++:

1. Cross-correlate the low-frequency content of Kick and Bass.
2. Decide whether Bass should be polarity-inverted.
3. Estimate the best sub-sample/integer delay.
4. Estimate how much low-frequency energy is recovered.
"""

from __future__ import annotations

import math

import numpy as np


SAMPLE_RATE = 48_000
MAX_LAG_MS = 20.0
ANALYSIS_LOW = 20.0
ANALYSIS_HIGH = 150.0


def make_kick(frequency: float = 50.0, duration: float = 0.35) -> np.ndarray:
    t = np.arange(int(SAMPLE_RATE * duration)) / SAMPLE_RATE
    envelope = np.exp(-t * 18.0)
    return np.sin(2.0 * math.pi * frequency * t) * envelope


def make_bass(frequency: float = 50.0, duration: float = 1.0) -> np.ndarray:
    t = np.arange(int(SAMPLE_RATE * duration)) / SAMPLE_RATE
    return np.sin(2.0 * math.pi * frequency * t) * 0.5


def pad_or_trim(signal: np.ndarray, length: int) -> np.ndarray:
    if len(signal) >= length:
        return signal[:length]
    return np.pad(signal, (0, length - len(signal)))


def shift(signal: np.ndarray, samples: int) -> np.ndarray:
    if samples == 0:
        return signal.copy()
    if samples > 0:
        return np.concatenate([np.zeros(samples, dtype=np.float32), signal[:-samples]])
    samples = -samples
    return np.concatenate([signal[samples:], np.zeros(samples, dtype=np.float32)])


def normalized_cross_correlation(bass: np.ndarray, kick: np.ndarray, lag: int) -> float:
    if lag >= 0:
        bass_segment = bass[lag:]
        kick_segment = kick[: len(bass_segment)]
    else:
        bass_segment = bass[:lag]
        kick_segment = kick[-lag:]

    if len(bass_segment) < 256:
        return 0.0

    bass_segment = bass_segment - np.mean(bass_segment)
    kick_segment = kick_segment - np.mean(kick_segment)

    denominator = math.sqrt(float(np.dot(bass_segment, bass_segment))
                           * float(np.dot(kick_segment, kick_segment)))
    if denominator < 1e-12:
        return 0.0

    return float(np.dot(bass_segment, kick_segment) / denominator)


def analyze(bass: np.ndarray, kick: np.ndarray, max_lag_ms: float = MAX_LAG_MS) -> dict:
    length = min(len(bass), len(kick))
    bass = pad_or_trim(bass, length).astype(np.float32)
    kick = pad_or_trim(kick, length).astype(np.float32)

    max_lag = int(SAMPLE_RATE * max_lag_ms / 1000.0)
    best_lag = 0
    best_corr = 0.0

    for lag in range(-max_lag, max_lag + 1):
        correlation = normalized_cross_correlation(bass, kick, lag)
        if (abs(correlation) > abs(best_corr) + 1e-9
            or (abs(correlation - best_corr) < 1e-9 and abs(lag) < abs(best_lag))):
            best_corr = correlation
            best_lag = lag

    invert_polarity = best_corr < 0.0
    delay_samples = -best_lag

    before = bass + kick
    corrected_bass = -bass if invert_polarity else bass
    corrected_bass = shift(corrected_bass, delay_samples)
    after = corrected_bass + kick

    before_energy = float(np.dot(before, before))
    after_energy = float(np.dot(after, after))
    cancellation_saved_db = 10.0 * math.log10(after_energy / max(before_energy, 1e-12))

    return {
        "best_lag": best_lag,
        "best_corr": best_corr,
        "invert_polarity": invert_polarity,
        "delay_samples": delay_samples,
        "cancellation_saved_db": cancellation_saved_db,
    }


def expect(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> None:
    t = np.arange(int(SAMPLE_RATE * 0.5)) / SAMPLE_RATE
    kick = np.sin(2.0 * math.pi * 50.0 * t).astype(np.float32)
    bass = (np.sin(2.0 * math.pi * 50.0 * t) * 0.5).astype(np.float32)

    aligned = analyze(bass, kick)
    print("aligned:", aligned)
    expect(abs(aligned["best_lag"]) <= 1, "aligned signals should have zero lag")
    expect(not aligned["invert_polarity"], "aligned signals should not be inverted")

    inverted = analyze(-bass, kick)
    print("inverted:", inverted)
    expect(inverted["invert_polarity"], "inverted Bass should be detected")
    expect(inverted["cancellation_saved_db"] > 1.0, "polarity fix should recover low-end energy")

    delayed_bass = shift(bass, 120)
    delayed = analyze(delayed_bass, kick)
    print("delayed 120 samples:", delayed)
    expect(abs(delayed["delay_samples"]) > 100, "time offset should be detected")

    print("Reference analysis checks passed.")


if __name__ == "__main__":
    main()
