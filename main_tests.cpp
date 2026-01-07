#include "Test.h"

bool test_linked_list_basic();
bool test_linked_queue_basic();
bool test_jsonlite_parse_sample();
bool test_integration_sample_invariants();
bool test_edge_no_renewables();

int main() {
  TestResult r;
  runTest("LinkedList basic", test_linked_list_basic, r);
  runTest("LinkedQueue basic", test_linked_queue_basic, r);
  runTest("JsonLite parse sample", test_jsonlite_parse_sample, r);
  runTest("Integration sample invariants", test_integration_sample_invariants, r);
  runTest("Edge case: no renewables", test_edge_no_renewables, r);

  std::cout << "\nPassed: " << r.passed << "  Failed: " << r.failed << "\n";
  return (r.failed == 0) ? 0 : 1;
}
