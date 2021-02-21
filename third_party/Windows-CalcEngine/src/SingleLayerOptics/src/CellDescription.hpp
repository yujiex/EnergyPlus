#ifndef BASECELLDESCRIPTION_H
#define BASECELLDESCRIPTION_H

namespace FenestrationCommon {

	enum class Side;

}

namespace SingleLayerOptics {

	class CBeamDirection;

	// Base interface for cell description.
	class ICellDescription {
	public:
		ICellDescription() {
		};
		virtual ~ICellDescription() = default;

		virtual double T_dir_dir( const FenestrationCommon::Side t_Side, const CBeamDirection& t_Direction ) = 0;
		virtual double R_dir_dir( const FenestrationCommon::Side t_Side, const CBeamDirection& t_Direction ) = 0;

        protected:
		ICellDescription &operator=(ICellDescription &&) = default;
		ICellDescription &operator=(const ICellDescription &) = default;
		ICellDescription(const ICellDescription &) = default;
		ICellDescription(ICellDescription &&) = default;
	};
}

#endif
