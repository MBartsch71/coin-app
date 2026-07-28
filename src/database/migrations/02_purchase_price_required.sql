-- Migration 02: purchase price becomes a required fact (ADR-0009)
-- 0.00 means "gift" or "unknown"; NULL is no longer permitted.

UPDATE coins SET purchase_price = 0.00 WHERE purchase_price IS NULL;

ALTER TABLE coins
    ALTER COLUMN purchase_price SET NOT NULL,
    ALTER COLUMN purchase_price SET DEFAULT 0.00;
