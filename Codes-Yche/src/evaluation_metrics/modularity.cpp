#include "modularity.h"

using namespace std;

int main(int argc, char *argv[]) {
  char *input_str = argv[1];
  char *output_str = argv[2];
  //    auto calculation = [](double left, double right) -> double {
  //        cout <<"Calculate Result:"<<1.0 / ((1 + exp(60 - 60 * left)) * ((1 +
  //        exp(60 - 60 * right))))<<endl;
  //        return 1.0 / ((1 + exp(2 - 1 * left)) * ((1 + exp(2 - 1 * right))));
  //    };

  //    auto calculation = [](double left, double right) -> double {
  //        return left * right;
  //    };

  //    auto calculation = [](double left, double right) -> double {
  //        return std::min(left, right);
  //    };

  auto calculation = [](double left, double right) -> double {
    return std::max(left, right);
  };
  using namespace yche;
  ModularityLinkBelonging<decltype(calculation)> modularity_calc(
      input_str, output_str, calculation);
  cout << modularity_calc.CalculateModularity();
  //    cout << "Hello, World!" << endl;
  return 0;
}
