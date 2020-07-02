# -------------------------------
# --- partial h grid for 2,2 ----
# -------------------------------

# a dtp based hgrid, with no unknowns! i just put "0" for the "?".
# used for experimentation with connection with lifetimes.

# written by: daniel rehman
# written on: 2006287.143949
# updated on: 2006287.143949


# ------ 0 --------

[     0
|    000 --> 0
]     0
.

[     1
|    000 --> 0
]     0
.

[     0
|    000 --> 1
]     1
.

# wtp
[     1
|    000 --> 1
]     1
.

# ------ 4 --------

[     0
|    010 --> 1
]     0
.

# unknown
[     1
|    010 --> 0
]     0
.

[     0
|    010 --> 1
]     1
.

# unknown
[     1
|    010 --> 0
]     1
.

# ------ 8 --------

[     0
|    100 --> 1
]     0
.

# unknown
[     1
|    100 --> 0
]     0
.

[     0
|    100 --> 1
]     1
.

# wtp unknown
[     1
|    100 --> 1
]     1
.


# ------ 12 --------

[     0
|    110 --> 1
]     0
.

# unknown
[     1
|    110 --> 0
]     0
.

[     0
|    110 --> 1
]     1
.

#  acc unknown
[     1
|    110 --> 0
]     1	
.


# -------------------------------------------------------------




# ------ 16 --------

[     0
|    001 --> 1
]     0
.

# unknown
[     1
|    001 --> 0
]     0
.

[     0
|    001 --> 1
]     1
.

# wtp unknown
[     1
|    001 --> 1
]     1
.

# ------ 20 --------

[     0
|    011 --> 1
]     0
.

# unknown
[     1
|    011 --> 1
]     0
.

[     0
|    011 --> 1
]     1
.

# acc unknown
[     1
|    011 --> 0
]     1
.

# ------ 24 --------

# wtp
[     0
|    101 --> 1
]     0
.


# unknown
[     1
|    101 --> 0
]     0
.

# wtp unknown
[     0
|    101 --> 1
]     1
.

# r diamond
[     1
|    101 --> 1
]     1
.


# ------ 28 --------


[     0
|    111 --> 1
]     0
.

[     1
|    111 --> 1
]     0
.

# unknown: (line breaker)
[     0
|    111 --> 1
]     1
.

[     1
|    111 --> 0
]     1	
.


# -------------- 32 --------------------

# done!



