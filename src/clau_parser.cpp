
#include "clau_parser.h"

ostream& operator<< (ostream& os, const Object& obj)
{
	const wiz::UserType& ut = ((const wiz::UserType&)obj);
	for (int i = 0; i < ut.GetListSize(); ++i) {
		//std::cout << "ItemList" << endl;
		if (ut.isLeaf()) {
			os << ut.GetList(i)->toString() << " ";
		}
		else {
			if (!((wiz::Type*)ut.GetList(i))->GetName().empty()) {
				os << ((wiz::Type*)ut.GetList(i))->GetName() << " = ";
			}

			os << "{\n";

			os << ut.GetList(i);

			os << "}";
			if (i != ut.GetListSize() - 1) {
				os << "\n";
			}
		}
	}
	return os;
}
