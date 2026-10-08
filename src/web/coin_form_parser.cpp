#include "web/coin_form_parser.hpp"

namespace web {

    namespace {

        auto get_str(const crow::query_string& form, const char* key)
            -> std::optional<std::string> {
                if (const char* v = form.get(key)) {
                    return std::string{v};
                }
                return std::nullopt;
            }

        auto get_nonempty(const crow::query_string& form, const char* key)
            -> std::optional<std::string> {
                auto v = get_str(form, key);
                if (v && !v->empty()) return v;
                return std::nullopt;
            }

        auto parse_int(const crow::query_string& form, const char* key) 
            -> std::optional<int> {
                auto v = get_str(form, key);
                if (!v) return std::nullopt;
                try {
                    return std::stoi(*v);
                } catch (const std::exception&) {
                    return std::nullopt;
                }
            }
    
        auto parse_double(const crow::query_string& form, const char* key)
            -> std::optional<double> {
                auto v = get_str(form, key);
                if (!v) return std::nullopt;
                try {
                    return std::stod(*v);
                } catch (const std::exception&) {
                    return std::nullopt;
                }
            }

        auto parse_i64(const crow::query_string& form, const char* key)
            -> std::optional<int64_t> {
                auto v = get_str(form, key);
                if (!v || v->empty()) return std::nullopt;
                try {
                    return std::stoll(*v);
                } catch (const std::exception&) {
                    return std::nullopt;        
                }
            }
        } // namespace

        auto parse_new_coin_form(const crow::query_string& form) 
            -> std::expected<coins::NewCoinData, std::string> {

                coins::NewCoinData data;

                if (auto ref = get_str(form, "reference_id")) {
                    if (!ref->empty()) {
                        try {
                            data.reference_id = std::stoll(*ref);
                        } catch (const std::exception&) {
                            return std::unexpected("Invalid reference_id");
                        }
                    }
                }

                if (!data.reference_id) {
                    data.ref_title   = get_nonempty(form, "ref_title");
                    data.ref_country = get_nonempty(form, "ref_country");
                    data.ref_metal   = get_nonempty(form, "ref_metal");
                }

                auto year = parse_int(form, "year");
                auto qty = parse_int(form, "quantity");
                auto price = parse_double(form, "purchase_price");

                if (!year || !qty  || !price) {
                    return std::unexpected("invalid or missing numeric fields");
                }

                data.year = *year;
                data.quantity = *qty;
                data.purchase_price = *price;
                data.purchase_currency = get_str(form, "purchase_currency").value_or("CHF");
                data.dealer = get_str(form, "dealer").value_or("");

                data.mint_mark = get_nonempty(form, "mint_mark");
                data.grade = get_nonempty(form, "grade");
                data.storage_location_id = parse_i64(form, "storage_location_id");

                return data;
            }
        
}