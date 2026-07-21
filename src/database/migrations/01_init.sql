-- 1. Create the ENUM for categorization
CREATE TYPE metal_type as ENUM ('Gold', 'Silver', 'Copper', 'Platinum', 'Palladium', 'Other');

-- 2. Storage locations
CREATE TABLE storage_locations (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    description TEXT,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- 3. Reference table (Catalog data)
CREATE TABLE coin_references (
    id SERIAL PRIMARY KEY,
    external_provider TEXT,    -- e.g. Numista, CoinGecko, etc.
    external_id TEXT,          -- e.g. Numista ID, CoinGecko ID
    title TEXT NOT NULL,  
    country TEXT NOT NULL,
    
    -- Metal Content
    primary_metal metal_type NOT NULL DEFAULT 'Silver',
    composition TEXT,
    fineness DECIMAL(6,4),
    
    -- Weights (grams)
    total_weight_g DECIMAL(12,3),
    fine_weight_g DECIMAL(12,3),

    diameter_mm DECIMAL(10,2),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- 4. Main collection table
CREATE TABLE coins (
    id SERIAL PRIMARY KEY,
    reference_id INTEGER REFERENCES coin_references(id),
    storage_location_id INTEGER REFERENCES storage_locations(id),
    
    title_override TEXT,
    year INTEGER NOT NULL,
    mint_mark TEXT,
    grade TEXT,
    quantity INTEGER NOT NULL DEFAULT 1,

    purchase_price DECIMAL(12,2),
    purchase_currency CHAR(3) DEFAULT 'CHF',
    purchase_date DATE,
    dealer TEXT,

    notes TEXT,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- 5. Tags
CREATE TABLE tags (
    id SERIAL PRIMARY KEY,
    name TEXT NOT NULL UNIQUE
);

-- 6. Coin Tags (Many-to-Many)
CREATE TABLE coin_tags (
    coin_id INTEGER REFERENCES coins(id) ON DELETE CASCADE,
    tag_id INTEGER REFERENCES tags(id) ON DELETE CASCADE,
    PRIMARY KEY (coin_id, tag_id)
);
