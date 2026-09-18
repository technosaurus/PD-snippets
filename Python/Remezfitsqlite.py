import sqlite3
import json
import numpy as np
from scipy.optimize import minimize_scalar

# 1. Core Remez Engine using SciPy
def remez_minimax_fit(func, x_min, x_max, degree, max_iter=20, tol=1e-6):
    """
    Finds the minimax polynomial coefficients using a 1-point Remez Exchange.
    Emulates the behavior of lolremez using SciPy scalar optimization.
    """
    n = degree
    # Initialize reference points using Chebyshev nodes (standard practice)
    nodes = 0.5 * (x_min + x_max) + 0.5 * (x_max - x_min) * np.cos(np.pi * np.arange(n + 2) / (n + 1))
    nodes = np.sort(nodes)

    for iteration in range(max_iter):
        # Step A: Solve the system of linear equations for the equiripple condition
        # P(x_i) + (-1)^i * E = f(x_i)
        A = np.zeros((n + 2, n + 2))
        B = np.zeros(n + 2)
        for i, x in enumerate(nodes):
            for j in range(n + 1):
                A[i, j] = x ** j
            A[i, n + 1] = (-1) ** i
            B[i] = func(x)
            
        # Solve for coefficients [c0, c1, ... cn] and error E
        solution = np.linalg.solve(A, B)
        coeffs = solution[:n + 1]
        
        # Define the error function: E(x) = f(x) - P(x)
        def neg_abs_error(x):
            poly_val = sum(c * (x ** k) for k, c in enumerate(coeffs))
            return -abs(func(x) - poly_val)

        # Step B: Locate the true maximum error point across the entire continuous domain
        # Scipy's minimize_scalar handles bounds flawlessly
        res = minimize_scalar(neg_abs_error, bounds=(x_min, x_max), method='bounded')
        max_err_x = res.x
        max_err_val = -res.fun

        # Check for convergence: if max error is close to our solved ripple error
        current_E = abs(solution[n + 1])
        if abs(max_err_val - current_E) < tol:
            return coeffs.tolist()

        # Step C: One-point exchange (replace the closest node sharing the same error sign)
        poly_val_at_max = sum(c * (max_err_x ** k) for k, c in enumerate(coeffs))
        max_err_sign = np.sign(func(max_err_x) - poly_val_at_max)
        
        # Find which reference point to swap out to keep error signs alternating
        best_swap_idx = 0
        min_dist = float('inf')
        for i, x in enumerate(nodes):
            poly_val_node = sum(c * (x ** k) for k, c in enumerate(coeffs))
            node_sign = np.sign(func(x) - poly_val_node)
            if node_sign == max_err_sign:
                dist = abs(x - max_err_x)
                if dist < min_dist:
                    min_dist = dist
                    best_swap_idx = i
                    
        nodes[best_swap_idx] = max_err_x
        nodes = np.sort(nodes)

    return coeffs.tolist()

# 2. Database Operations Layer
def store_property_curve(db_path, entity_name, property_key, ind_var, x_min, x_max, coefficients):
    """
    Inserts or updates a piecewise approximation interval inside SQLite.
    """
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Get or create the unique entity ID
    cursor.execute("SELECT entity_id FROM entities WHERE name = ?", (entity_name,))
    row = cursor.fetchone()
    if row:
        entity_id = row[0]
    else:
        cursor.execute("INSERT INTO entities (name, type) VALUES (?, 'Chemical')", (entity_name,))
        entity_id = cursor.lastrowid

    # Serialize coefficients to clean JSON
    coeffs_json = json.dumps(coefficients)

    # Insert the curve segment into our ranges table
    cursor.execute("""
        INSERT INTO property_curves (entity_id, property_key, independent_variable, param_min, param_max, coefficients)
        VALUES (?, ?, ?, ?, ?, ?)
    """, (entity_id, property_key, ind_var, x_min, x_max, coeffs_json))

    conn.commit()
    conn.close()
    print(f" Successfully archived {property_key} approximation curve for {entity_name}!")

# --- Execution Simulation ---
if __name__ == "__main__":
    # Let's mock a physical property curve function (e.g., dynamic viscosity of an experimental fluid)
    # Inside your real script, replace this with a scipy spline interpolation of your raw spreadsheet data points
    def experimental_viscosity_curve(temp_kelvin):
        return 0.05 + (200 / temp_kelvin) + 1.2e-5 * (temp_kelvin ** 2)

    # 1. Compute a 4th-degree minimax Remez polynomial over the target window (250K to 500K)
    print("Calculating optimal minimax polynomial via SciPy...")
    optimized_coefficients = remez_minimax_fit(
        func=experimental_viscosity_curve, 
        x_min=250.0, 
        x_max=500.0, 
        degree=4
    )
    
    print(f"Calculated Coefficients (c0 to c4): {optimized_coefficients}")

    # 2. Save seamlessly to your master SQLite scientific file
    store_property_curve(
        db_path="research_knowledge_base.db",
        entity_name="Experimental Solvent Alpha",
        property_key="viscosity",
        ind_var="temperature_kelvin",
        x_min=250.0,
        x_max=500.0,
        coefficients=optimized_coefficients
  )
  
