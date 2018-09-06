#include <Rcpp.h>
using namespace Rcpp;

//' Generate predictions of inside temperature
//'
//' @param input_vct List or DF of vectorized input parameters. `t_out`, `heat_gain`.
//' @param input_ind List or DF of parameters with length one `init_t_room`.
//' @param hloss heat loss of the building.
//' @param sheat specific heat of the building.
//' @param expand more detailed resuts.
//' @export
// [[Rcpp::export]]
DataFrame heat_model(List input_vct,
                     List input_ind,
                     float hloss,
                     float sheat,
                     bool expand = false) {

  // Great, it seems that Rcpp accepts can accept data frames as lists
  std::vector<double> t_out = input_vct["t_out"];
  std::vector<double> heat_gain = input_vct["heat_gain"];

  double init_t_room = input_ind["init_t_room"];

  // initialize vectors
  int n = t_out.size();

  std::vector<double> init_e(n);
  // std::vector<double> more_e(n);
  std::vector<double> end_e(n);
  std::vector<double> init_t(n);
  // std::vector<double> more_t(n);
  std::vector<double> end_t(n);

  std::vector<double> init_dt(n);
  std::vector<double> end_dt(n);

  std::vector<double> init_loss(n);
  std::vector<double> end_loss(n);

  std::vector<double> good_loss(n);
  std::vector<double> good_e(n);
  std::vector<double> good_t(n);

  // define the starting energy and temperature
  good_e[0] = init_t_room * sheat;
  good_t[0] = init_t_room;

  for (int i=1; i < n; ++i){

    // the initial and final energy content in the building
    init_e[i] = good_e[i-1];
    // more_e[i] = heat_kwh[i];
    end_e[i] = good_e[i-1] + heat_gain[i];

    // the initial and final temperatures
    init_t[i] = init_e[i] / sheat;
    // more_t[i] = more_e[i] / sheat;
    end_t[i] = end_e[i] / sheat;

    // the initial and final changes in temperature
    init_dt[i] = init_t[i] - t_out[i];
    end_dt[i] = end_t[i] - t_out[i];

    // the loss of energy is *almost* the same of the final theoretical temperature
    // without considering the losses (in theory)
    init_loss[i] = init_dt[i] * hloss;
    end_loss[i] = end_dt[i] * hloss;
    good_loss[i] = init_loss[i] + (end_loss[i] - init_loss[i]) * (1/(end_loss[i]/init_loss[i]));

    // the corrected energy in the building considers the losses.
    // the corrected temperature comes directly from this energy.
    good_e[i] = end_e[i] - good_loss[i];
    good_t[i] = good_e[i] / sheat;

  }

  List res = DataFrame::create(_["t_room"]= good_t);

  if (expand == true) {
    res.push_back(good_loss, "heat_loss");
    res.push_back(good_e, "heat_content");
  }

  return res;
}
