#include "domain.h"


namespace catalogue {

	bool CompareBuses::operator()(const Bus* l, const Bus* r) const {
		return std::lexicographical_compare(l->name.begin(), l->name.end(),
						    r->name.begin(), r->name.end());
	}

	bool CompareStop::operator()(const Stop* l, const Stop* r) const {
		return std::lexicographical_compare(l->name.begin(), l->name.end(),
						    r->name.begin(), r->name.end());
	}

}
